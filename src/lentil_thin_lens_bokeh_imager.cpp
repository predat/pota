#include <ai.h>
#include <vector>
#include <iostream>
#include <map>
#include "lentil_thinlens.h"
#include "lentil.h"

// currently this works by searching for a node with specific name "lentil_filter", not ideal.

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(ThinLensBokehImagerMtd);

struct LentilImagerData {
    AtString camera_node_type;
    AtString lentil_thinlens_string;
    AtString lentil_po_string;
    AtNode *camera_node;
};

node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
}
 
node_initialize
{
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AiRenderSetHintInt(AtString("imager_schedule"), AI_DRIVER_SCHEDULE_FULL);

  
  const AtNodeEntry *bokeh_filter = AiNodeEntryLookUp("lentil_thin_lens_bokeh_filter");
  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_filter");
  LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);

  if (filter_data->enabled) AiMsgInfo("[LENTIL BIDIRECTIONAL TL] Starting Imager.");

  int aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
  if (aa_samples < 3) {
    return;
  }
}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open {}
 
driver_extension
{
   static const char *extensions[] = {"exr", NULL};
   return extensions;
}
 
driver_needs_bucket
{
   return true; // API: true if the bucket needs to be rendered, false if it can be skipped
}
 
driver_prepare_bucket {} // called before a bucket is rendered


 
driver_process_bucket {
  LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);

  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_filter");
  LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);

  
  if (!filter_data->enabled) return;
  
  const double xres = (double)filter_data->xres;
  const double yres = (double)filter_data->yres;

  const char *aov_name_cstr = 0;
  int aov_type = 0;
  const void *bucket_data;
  
  // Iterate over all the AOVs hooked up to this driver
  while (AiOutputIteratorGetNext(iterator, &aov_name_cstr, &aov_type, &bucket_data)){
    std::string aov_name_str = aov_name_cstr;
    AtString aov_name_current = AtString(aov_name_cstr);
    if (std::find(filter_data->aov_list_name.begin(), filter_data->aov_list_name.end(),AtString(aov_name_cstr))!=filter_data->aov_list_name.end()){
      if (aov_name_current == AtString("transmission")) continue;
      AiMsgInfo("[LENTIL] imager looping over: %s", aov_name_str.c_str());
      AtString aov_name = AtString(aov_name_str.c_str());

      for (int j = 0; j < bucket_size_y; ++j) {
        for (int i = 0; i < bucket_size_x; ++i) {
          int y = j + bucket_yo;
          int x = i + bucket_xo;
          int in_idx = j * bucket_size_x + i;
          int linear_pixel = x + (y * xres);

          // only rgba/rgb aovs have been guassian filtered, so need to normalize only them
          if (aov_type == AI_TYPE_RGBA || aov_type == AI_TYPE_RGB){

            AtRGBA redist = filter_data->image_redist[aov_name][linear_pixel] / ((filter_data->redist_weight_per_pixel[aov_name][linear_pixel] == 0.0) ? 1.0 : filter_data->redist_weight_per_pixel[aov_name][linear_pixel]);
            AtRGBA unredist = filter_data->image_unredist[aov_name][linear_pixel] / ((filter_data->unredist_weight_per_pixel[aov_name][linear_pixel] == 0.0) ? 1.0 : filter_data->unredist_weight_per_pixel[aov_name][linear_pixel]);
            AtRGBA combined_redist_unredist = (unredist * (1.0-filter_data->redist_weight_per_pixel[aov_name][linear_pixel])) + (redist * (filter_data->redist_weight_per_pixel[aov_name][linear_pixel]));
            
            // this currently doesn't work for the rgb layers because alpha is wrong for rgb layers
            if (combined_redist_unredist.a > 0.95) combined_redist_unredist /= combined_redist_unredist.a;


            ((AtRGBA*)bucket_data)[in_idx] = combined_redist_unredist;
            
          } 
          else {
            ((AtRGBA*)bucket_data)[in_idx] = filter_data->image[aov_name][linear_pixel];
          }
        }
      }
    }
    
  }
}


driver_write_bucket {}
 
driver_close {}
 
node_finish {
  LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);
  delete imager_data;
}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) ThinLensBokehImagerMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "lentil_thin_lens_bokeh_imager";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 