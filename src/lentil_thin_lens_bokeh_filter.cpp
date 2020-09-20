#include <ai.h>
#include <vector>
#include <iostream>
#include <map>
#include "lentil_thinlens.h"

 
AI_FILTER_NODE_EXPORT_METHODS(ThinLensBokehFilterMtd);
 


inline float thinlens_get_image_dist_focusdist(CameraThinLens *tl){
    return (-tl->focal_length * -tl->focus_distance) / (-tl->focal_length + -tl->focus_distance);
}

inline float thinlens_get_coc(AtVector sample_pos_ws, ThinLensBokehDriver *bokeh, CameraThinLens *tl){
  // world to camera space transform, static just for CoC
  AtMatrix world_to_camera_matrix_static;
  float time_middle = linear_interpolate(0.5, bokeh->time_start, bokeh->time_end);
  AiWorldToCameraMatrix(AiUniverseGetCamera(), time_middle, world_to_camera_matrix_static);
  const AtVector camera_space_sample_position_static = AiM4PointByMatrixMult(world_to_camera_matrix_static, sample_pos_ws); // just for CoC size calculation
  
  const float image_dist_samplepos = (-tl->focal_length * camera_space_sample_position_static.z) / (-tl->focal_length + camera_space_sample_position_static.z);
  const float image_dist_focusdist = thinlens_get_image_dist_focusdist(tl);
  return std::abs((tl->aperture_radius * (image_dist_samplepos - image_dist_focusdist))/image_dist_samplepos); // coc diameter
}



node_parameters {}
 
node_initialize
{
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGBA transmission", "RGBA lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, true, required_aovs);
  AiNodeSetLocalData(node, new ThinLensBokehDriver());
}
 
node_update 
{
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());


  const AtNodeEntry *nentry = AiNodeGetNodeEntry(node);
  if (AiNodeEntryGetCount(nentry) > 1){
    AiMsgError("[LENTIL BIDIRECTIONAL ERROR]: Multiple nodes of type lentil_thin_lens_bokeh_filter exist. "
               "All of bidirectional AOVs should be connected to a single lentil_thin_lens_bokeh_filter node. "
               "This is to avoid doing the bidirectional sampling multiple times.");
  }

  // THIS IS DOUBLE CODE, also in camera!
  // get camera params & recompute the node_update section to avoid race condition when sharing datastruct
  AtNode *cameranode = AiUniverseGetCamera();
  #include "node_update_thinlens.h"

  bokeh->enabled = true;


  if (tl->enable_dof == false) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] Depth of field is disabled, therefore disabling Bidirectional sampling.");
    bokeh->enabled = false;
    return;
  }


  // don't compute for interactive previews
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
  bokeh->min_aa_samples = 3;
  if (bokeh->aa_samples < bokeh->min_aa_samples) {
    bokeh->enabled = false;
  }


  // disable for non-lentil cameras
  if (!AiNodeIs(cameranode, AtString("lentil_thinlens"))) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] Camera is not of type lentil_thinlens");
    bokeh->enabled = false;
    return;
  }

  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = 2.0;
  bokeh->rgba_string = AtString("RGBA");
  bokeh->framenumber = static_cast<int>(AiNodeGetFlt(AiUniverseGetOptions(), "frame"));

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);

  bokeh->time_start = AiCameraGetShutterStart();
  bokeh->time_end = AiCameraGetShutterEnd();


  if (tl->bidir_sample_mult == 0) bokeh->enabled = false;
  if (bokeh->enabled) AiMsgInfo("[LENTIL BIDIRECTIONAL TL] Starting bidirectional sampling.");



// ====> this is shitty, don't have the iterator here yet (used to be in the driver..)
// ====> how should i loop over the aovs?

  // get name/type of connected aovs
  // const char *name = 0;
  // int pixelType = 0;
  // bokeh->aov_list_name.clear();
  // bokeh->aov_list_type.clear();
  // while(AiOutputIteratorGetNext(iterator, &name, &pixelType, 0)){
  //   // if aov already considered (can happen due to multiple drivers considering the same aov, such as kick_display and driver_exr)
  //   if (std::find(bokeh->aov_list_name.begin(), bokeh->aov_list_name.end(), AtString(name)) != bokeh->aov_list_name.end()) continue;
    
  //   bokeh->aov_list_name.push_back(AtString(name));
  //   bokeh->aov_list_type.push_back(pixelType); 
  //   bokeh->image[AtString(name)].clear();
  //   bokeh->image[AtString(name)].resize(bokeh->xres * bokeh->yres);
  //   bokeh->image_redist[AtString(name)].clear();
  //   bokeh->image_redist[AtString(name)].resize(bokeh->xres * bokeh->yres);
  //   bokeh->image_unredist[AtString(name)].clear();
  //   bokeh->image_unredist[AtString(name)].resize(bokeh->xres * bokeh->yres);
  //   bokeh->redist_weight_per_pixel[AtString(name)].clear();
  //   bokeh->redist_weight_per_pixel[AtString(name)].resize(bokeh->xres * bokeh->yres);
  //   bokeh->unredist_weight_per_pixel[AtString(name)].clear();
  //   bokeh->unredist_weight_per_pixel[AtString(name)].resize(bokeh->xres * bokeh->yres);
  // }
  // AiOutputIteratorReset(iterator);


  // just filled with test data, change to something like above.
  bokeh->aov_list_name.clear();
  bokeh->aov_list_type.clear();
  bokeh->aov_list_name.push_back(AtString("RGBA"));
  bokeh->aov_list_type.push_back(AI_TYPE_RGBA); 
  bokeh->image[AtString("RGBA")].clear();
  bokeh->image[AtString("RGBA")].resize(bokeh->xres * bokeh->yres);
  bokeh->image_redist[AtString("RGBA")].clear();
  bokeh->image_redist[AtString("RGBA")].resize(bokeh->xres * bokeh->yres);
  bokeh->image_unredist[AtString("RGBA")].clear();
  bokeh->image_unredist[AtString("RGBA")].resize(bokeh->xres * bokeh->yres);
  bokeh->redist_weight_per_pixel[AtString("RGBA")].clear();
  bokeh->redist_weight_per_pixel[AtString("RGBA")].resize(bokeh->xres * bokeh->yres);
  bokeh->unredist_weight_per_pixel[AtString("RGBA")].clear();
  bokeh->unredist_weight_per_pixel[AtString("RGBA")].resize(bokeh->xres * bokeh->yres);





  AiFilterUpdate(node, 1.65);
}
 

filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      default:
         return AI_TYPE_NONE;
   }
}

 
filter_pixel
{
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (bokeh->enabled) {
    const double xres = (double)bokeh->xres;
    const double yres = (double)bokeh->yres;
    const double frame_aspect_ratio = xres/yres;

    while (AiAOVSampleIteratorGetNext(iterator)) {

      int px, py;
      AiAOVSampleIteratorGetPixel(iterator, px, py);

      bool redistribute = true;
      bool partly_redistributed = false;

      AtRGBA sample = AiAOVSampleIteratorGetRGBA(iterator);
      const AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(iterator, AtString("P"));
      float depth = AiAOVSampleIteratorGetAOVFlt(iterator, AtString("Z")); // what to do when values are INF?
      const float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      if (inv_density <= 0.f) continue; // does this every happen? test
      const float filter_width_half = std::ceil(bokeh->filter_width * 0.5);
      
      const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(iterator, AtString("transmission"));
      bool transmitted_energy_in_sample = (AiColorMaxRGB(sample_transmission) > 0.0);
      if (transmitted_energy_in_sample){
        sample.r -= sample_transmission.r;
        sample.g -= sample_transmission.g;
        sample.b -= sample_transmission.b;
      }

      const float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;
      if (sample_luminance < tl->bidir_min_luminance) redistribute = false;
      if (!std::isfinite(depth)) redistribute = false; // not sure if this works.. Z AOV has inf values at skydome hits
      if (AiV3IsSmall(sample_pos_ws)) redistribute = false; // not sure if this works .. position is 0,0,0 at skydome hits
      if (AiAOVSampleIteratorHasAOVValue(iterator, AtString("lentil_bidir_ignore"), AI_TYPE_RGBA)) redistribute = false;
      


    // ENERGY REDISTRIBUTION
      if (redistribute) {
        
        // additional luminance with soft transition
        float fitted_bidir_add_luminance = 0.0;
        if (tl->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, tl->bidir_add_luminance, tl->bidir_add_luminance_transition, tl->bidir_min_luminance);
        

        float circle_of_confusion = thinlens_get_coc(sample_pos_ws, bokeh, tl);
        const float coc_squared_pixels = std::pow(circle_of_confusion * bokeh->yres, 2) * tl->bidir_sample_mult * 0.01; // pixel area as baseline for sample count
        // if (std::pow(circle_of_confusion * bokeh->yres, 2) < std::pow(15, 2)) goto no_redist; // 15^2 px minimum coc
        int samples = std::ceil(coc_squared_pixels / (double)std::pow(bokeh->aa_samples, 2)); // aa_sample independence
        samples = std::clamp(samples, 100, 1000000); // not sure if a million is actually ever hit..


        unsigned int total_samples_taken = 0;
        unsigned int max_total_samples = samples*5;

        for(int count=0; count<samples && total_samples_taken < max_total_samples; count++) {
          ++total_samples_taken;
          unsigned int seed = tea<8>(px*py+px, total_samples_taken);

          // world to camera space transform, motion blurred
          AtMatrix world_to_camera_matrix_motionblurred;
          float currenttime = linear_interpolate(rng(seed), bokeh->time_start, bokeh->time_end); // should I create new random sample, or can I re-use another one?
          AiWorldToCameraMatrix(AiUniverseGetCamera(), currenttime, world_to_camera_matrix_motionblurred);
          AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, sample_pos_ws);
          float image_dist_samplepos_mb = (-tl->focal_length * camera_space_sample_position_mb.z) / (-tl->focal_length + camera_space_sample_position_mb.z);

          // either get uniformly distributed points on the unit disk or bokeh image
          Eigen::Vector2d unit_disk(0, 0);
          if (tl->bokeh_enable_image) tl->image.bokehSample(rng(seed),rng(seed), unit_disk, rng(seed), rng(seed));
          else if (tl->bokeh_aperture_blades < 2) concentricDiskSample(rng(seed),rng(seed), unit_disk, tl->abb_spherical, tl->circle_to_square, tl->bokeh_anamorphic);
          else lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), rng(seed),rng(seed), 1.0, tl->bokeh_aperture_blades);



          unit_disk(0) *= tl->bokeh_anamorphic;
          AtVector lens(unit_disk(0) * tl->aperture_radius, unit_disk(1) * tl->aperture_radius, 0.0);


          // aberration inputs
          float abb_field_curvature = 0.0;


          // ray through center of lens
          AtVector dir_from_center = AiV3Normalize(camera_space_sample_position_mb);
          AtVector dir_lens_to_P = AiV3Normalize(camera_space_sample_position_mb - lens);
          // perturb ray direction to simulate coma aberration
          // todo: the bidirectional case isn't entirely the same as the forward case.. fix!
          // current strategy is to perturb the initial sample position by doing the same ray perturbation i'm doing in the forward case
          float abb_coma = tl->abb_coma * abb_coma_multipliers(tl->sensor_width, tl->focal_length, dir_from_center, unit_disk);
          dir_lens_to_P = abb_coma_perturb(dir_lens_to_P, dir_from_center, abb_coma, true);
          camera_space_sample_position_mb = AiV3Length(camera_space_sample_position_mb) * dir_lens_to_P;
          dir_from_center = AiV3Normalize(camera_space_sample_position_mb);

          float samplepos_image_intersection = std::abs(image_dist_samplepos_mb/dir_from_center.z);
          AtVector samplepos_image_point = dir_from_center * samplepos_image_intersection;


          // depth of field
          AtVector dir_from_lens_to_image_sample = AiV3Normalize(samplepos_image_point - lens);





          float focusdist_intersection = std::abs(thinlens_get_image_dist_focusdist(tl)/dir_from_lens_to_image_sample.z);
          AtVector focusdist_image_point = lens + dir_from_lens_to_image_sample*focusdist_intersection;
          
          // bring back to (x, y, 1)
          AtVector2 sensor_position(focusdist_image_point.x / focusdist_image_point.z,
                                    focusdist_image_point.y / focusdist_image_point.z);
          // transform to screenspace coordinate mapping
          sensor_position /= (tl->sensor_width*0.5)/-tl->focal_length;


          // optical vignetting
          dir_lens_to_P = AiV3Normalize(camera_space_sample_position_mb - lens);
          if (tl->optical_vignetting_distance > 0.0){
            // if (image_dist_samplepos<image_dist_focusdist) lens *= -1.0; // this really shouldn't be the case.... also no way i can do that in forward tracing?
            if (!empericalOpticalVignettingSquare(lens, dir_lens_to_P, tl->aperture_radius, tl->optical_vignetting_radius, tl->optical_vignetting_distance, lerp_squircle_mapping(tl->circle_to_square))){
                --count;
                continue;
            }
          }


          // barrel distortion (inverse)
          if (tl->abb_distortion > 0.0) sensor_position = inverseBarrelDistortion(AtVector2(sensor_position.x, sensor_position.y), tl->abb_distortion);
          

          // convert sensor position to pixel position
          Eigen::Vector2d s(sensor_position.x, sensor_position.y * frame_aspect_ratio);
          const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
          const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

          // if outside of image
          if ((pixel_x >= xres) || (pixel_x < 0) || (pixel_y >= yres) || (pixel_y < 0)) {
            --count; // much room for improvement here, potentially many samples are wasted outside of frame, could keep track of a bbox
            continue;
          }

          // write sample to image
          unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel_y) + floor(pixel_x));

          // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

          for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
            add_to_buffer(sample, pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                          samples, inv_density, fitted_bidir_add_luminance, depth, iterator,
                          bokeh->image_redist, bokeh->redist_weight_per_pixel, bokeh->image, bokeh->zbuffer, 
                          bokeh->rgba_string);
          
          }
        }


        if (transmitted_energy_in_sample) {
          partly_redistributed = true;
          goto no_redist;
        }
      }

      else { // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
      no_redist:
      
        if (transmitted_energy_in_sample && partly_redistributed) {
          sample.r = sample_transmission.r;
          sample.g = sample_transmission.g;
          sample.b = sample_transmission.b;
        } else if (transmitted_energy_in_sample && !partly_redistributed) {
          sample.r += sample_transmission.r;
          sample.g += sample_transmission.g;
          sample.b += sample_transmission.b;
        }



        // loop over all pixels in filter radius, then compute the filter weight based on the offset not to the original pixel (px, py), but the filter pixel (x, y)
        for (unsigned y = py - filter_width_half; y <= py + filter_width_half; y++) {
          for (unsigned x = px - filter_width_half; x <= px + filter_width_half; x++) {
            
            if (y < 0 || y >= bokeh->yres) continue; // edge fix
            if (x < 0 || x >= bokeh->xres) continue; // edge fix

            const unsigned pixelnumber = static_cast<int>(bokeh->xres * y + x);
            
            const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(iterator); // offset within original pixel
            AtVector2 subpixel_pos_dist = AtVector2((px+subpixel_position.x) - x, (py+subpixel_position.y) - y);
            float filter_weight = filter_gaussian(subpixel_pos_dist, bokeh->filter_width);
            if (filter_weight == 0) continue;


            for (size_t i=0; i<bokeh->aov_list_name.size(); i++){
              add_to_buffer(sample, pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                            1, inv_density, 0.0, depth, iterator,
                            bokeh->image_unredist, bokeh->unredist_weight_per_pixel, bokeh->image, bokeh->zbuffer, 
                            bokeh->rgba_string);
            }


          }
        }
      }
    }
  }


  // do regular filtering (passthrough) for display purposes
  AiAOVSampleIteratorReset(iterator);
  const float width = 1.65;
  float aweight = 0.0f;
  AtRGBA avalue = AI_RGBA_ZERO;

  while (AiAOVSampleIteratorGetNext(iterator))
  {
      // take into account adaptive sampling
      float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      if (inv_density <= 0.f) continue;

      // determine distance to filter center
      const AtVector2& offset = AiAOVSampleIteratorGetOffset(iterator);
      const float r = AiSqr(2 / width) * (AiSqr(offset.x) + AiSqr(offset.y));
      if (r > 1.0f)
          continue;

      // gaussian filter weight
      const float weight = AiFastExp(2 * -r) * inv_density;

      // accumulate weights and colors
      AtRGBA sample_energy = AiAOVSampleIteratorGetRGBA(iterator);
      const float sample_luminance = sample_energy.r*0.21 + sample_energy.g*0.71 + sample_energy.b*0.072;

      
      avalue += weight * sample_energy;
      aweight += weight;
  }
 
   // compute final filtered color
   if (aweight != 0.0f) avalue /= aweight;
   *((AtRGBA*)data_out) = avalue;

}
 
 
node_finish
{
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  delete bokeh;
}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) ThinLensBokehFilterMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "lentil_thin_lens_bokeh_filter";
  node->node_type = AI_NODE_FILTER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 