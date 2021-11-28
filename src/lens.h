#pragma once
#include <cmath>
#include <vector>
#include <algorithm>
#include "../../Eigen/Eigen/Core"
#include "../../Eigen/Eigen/LU"

#include "global.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

static inline double raytrace_dot(Eigen::Vector3d u, Eigen::Vector3d v) {
  return u(0)*v(0) + u(1)*v(1) + u(2)*v(2);
}

static inline void raytrace_cross(Eigen::Vector3d &r, const Eigen::Vector3d u, const Eigen::Vector3d v) {
  r(0) = u(1)*v(2)-u(2)*v(1);
  r(1) = u(2)*v(0)-u(0)*v(2);
  r(2) = u(0)*v(1)-u(1)*v(0);
}

static inline void raytrace_normalise(Eigen::Vector3d &v) {
  const double ilen = 1.0f/std::sqrt(raytrace_dot(v,v));
  for(int k=0;k<3;k++) v(k) *= ilen;
}

static inline void raytrace_substract(Eigen::Vector3d &u, const Eigen::Vector3d v) {
  for(int k = 0; k < 3; k++) u(k) -= v(k);
}

static inline void raytrace_multiply(Eigen::Vector3d &v, const double s) {
  for(int k = 0; k < 3; k++) v(k) *= s;
}

static inline void propagate(Eigen::Vector3d &pos, const Eigen::Vector3d dir, const double dist) {
  for(int i=0;i<3;i++) pos(i) += dir(i) * dist;
}


static inline void planeToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double planepos) {
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = planepos;

  outdir(0) = indir(0);
  outdir(1) = indir(1);
  outdir(2) = 1;

  raytrace_normalise(outdir);
}

static inline void csToPlane(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double planepos)
{
  //intersection with plane at z = planepos
  const double t = (planepos - inpos(2)) / indir(2);

  outpos(0) = inpos(0) + t * indir(0);
  outpos(1) = inpos(1) + t * indir(1);

  outdir(0) = indir(0) / std::abs(indir(2));
  outdir(1) = indir(1) / std::abs(indir(2));
}

static inline void sphereToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double center, const double sphereRad)
{
  const Eigen::Vector3d normal(
    inpos(0)/sphereRad,
    inpos(1)/sphereRad,
    std::sqrt(std::max(0.0, sphereRad*sphereRad-inpos(0)*inpos(0)-inpos(1)*inpos(1)))/std::abs(sphereRad)
  );

  const Eigen::Vector3d tempDir(
    indir(0),
    indir(1),
    std::sqrt(std::max(0.0, 1.0-indir(0)*indir(0)-indir(1)*indir(1)))
  );

  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex);
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);

  outdir(0) = tempDir(0) * ex(0) + tempDir(1) * ey(0) + tempDir(2) * normal(0);
  outdir(1) = tempDir(0) * ex(1) + tempDir(1) * ey(1) + tempDir(2) * normal(1);
  outdir(2) = tempDir(0) * ex(2) + tempDir(1) * ey(2) + tempDir(2) * normal(2);

  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = normal(2) * sphereRad + center;
}

static inline void csToSphere(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double sphereCenter, const double sphereRad)
{
  const Eigen::Vector3d normal(
    inpos(0)/sphereRad,
    inpos(1)/sphereRad,
    std::abs((inpos(2)-sphereCenter)/sphereRad)
  );

  Eigen::Vector3d tempDir(indir(0), indir(1), indir(2));
  raytrace_normalise(tempDir);

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex);
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  
  // encode ray direction as projected position on unit disk perpendicular to the normal
  outdir(0) = raytrace_dot(tempDir, ex);
  outdir(1) = raytrace_dot(tempDir, ey);

  // outpos is unchanged, z term omitted
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
}


static inline void csToCylinder(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double center, const double R, const bool cyl_y) {

  Eigen::Vector3d normal(0,0,0);
  if (cyl_y){
    normal(0) = inpos(0)/R;
    normal(2) = std::abs((inpos(2) - center)/R);
  } else {
    normal(1) = inpos(1)/R;
    normal(2) = std::abs((inpos(2) - center)/R);
  }

  Eigen::Vector3d tempDir(indir(0), indir(1), indir(2));
  raytrace_normalise(tempDir); //untested

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  raytrace_normalise(ey); // not sure if this is necessary
  
  // encode ray direction as projected position on unit disk perpendicular to the normal
  outdir(0) = raytrace_dot(tempDir, ex);
  outdir(1) = raytrace_dot(tempDir, ey);

  // outpos is unchanged, z term omitted
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
}


static inline void cylinderToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double center, const double R, const bool cyl_y) {

  Eigen::Vector3d normal(0,0,0);
  if (cyl_y){
    normal(0) = inpos(0)/R;
    normal(2) = std::sqrt(std::max(0.0, R*R-inpos(0)*inpos(0)))/std::abs(R);
  } else {
    normal(1) = inpos(1)/R;
    normal(2) = std::sqrt(std::max(0.0, R*R-inpos(1)*inpos(1)))/std::abs(R);
  }

  const Eigen::Vector3d tempDir(
    indir(0), 
    indir(1), 
    std::sqrt(std::max(0.0, 1.0-indir(0)*indir(0)-indir(1)*indir(1)))
  );

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex); // not sure if this is necessary
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  raytrace_normalise(ey); // not sure if this is necessary
  
  outdir(0) = tempDir(0) * ex(0) + tempDir(1) * ey(0) + tempDir(2) * normal(0);
  outdir(1) = tempDir(0) * ex(1) + tempDir(1) * ey(1) + tempDir(2) * normal(1);
  outdir(2) = tempDir(0) * ex(2) + tempDir(1) * ey(2) + tempDir(2) * normal(2);

  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = normal(2) * R + center;
}



// helper function for dumped polynomials to compute integer powers of x:
static inline double lens_ipow(const double x, const int exp) {
  if(exp == 0) return 1.0f;
  if(exp == 1) return x;
  if(exp == 2) return x*x;
  const double p2 = lens_ipow(x, exp/2);
  if(exp &  1) return x * p2 * p2;
  return p2 * p2;
}


inline void load_lens_constants (Camera *camera)
{
  switch (camera->lensModel){
    #include "../include/auto_generated_lens_includes/load_lens_constants.h"
  }
}

// evaluates from sensor (in) to outer pupil (out).
// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
// two-plane parametrization (that is the third component of the direction would be 1.0).
// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
// returns the transmittance computed from the polynomial.
static inline double lens_evaluate(const Eigen::VectorXd in, Eigen::VectorXd &out, Camera *camera)
{
  const double x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
  double out_transmittance = 0.0;
  switch (camera->lensModel){
    #include "../include/auto_generated_lens_includes/load_pt_evaluate.h"
  }

  return std::max(0.0, out_transmittance);
}

/*
// evaluates from the sensor (in) to the aperture (out) only
// returns the transmittance.
static inline float lens_evaluate_aperture(const float *in, float *out)
{
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_aperture.h"
  out[0] = out_x; out[1] = out_y; out[2] = out_dx; out[3] = out_dy;
  return std::max(0.0f, out_transmittance);
}
*/


// solves for the two directions [dx,dy], keeps the two positions [x,y] and the
// wavelength, such that the path through the lens system will be valid, i.e.
// lens_evaluate_aperture(in, out) will yield the same out given the solved for in.
// in: point on sensor. out: point on aperture.
static inline void lens_pt_sample_aperture(Eigen::VectorXd &in, Eigen::VectorXd &out, double dist, Camera *camera)
{
  double out_x = out[0], out_y = out[1], out_dx = out[2], out_dy = out[3], out_transmittance = 1.0f;
  double x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];

  switch (camera->lensModel){
    #include "../include/auto_generated_lens_includes/load_pt_sample_aperture.h"
  }

  // directions may have changed, copy all to be sure.
  out[0] = out_x; // dont think this is needed
  out[1] = out_y; // dont think this is needed
  out[2] = out_dx;
  out[3] = out_dy;

  in[0] = x; // dont think this is needed
  in[1] = y; // dont think this is needed
  in[2] = dx;
  in[3] = dy;
}



// solves for a sensor position given a scene point and an aperture point
// returns transmittance from sensor to outer pupil
static inline double lens_lt_sample_aperture(
    const Eigen::Vector3d scene,    // 3d point in scene in camera space
    const Eigen::Vector2d ap,       // 2d point on aperture (in camera space, z is known)
    Eigen::VectorXd &sensor,        // output point and direction on sensor plane/plane
    Eigen::VectorXd &out,           // output point and direction on outer pupil
    const double lambda,            // wavelength
    Camera *camera)   
{
  const double scene_x = scene[0], scene_y = scene[1], scene_z = scene[2];
  const double ap_x = ap[0], ap_y = ap[1];
  double x = 0, y = 0, dx = 0, dy = 0;

  switch (camera->lensModel){
    #include "../include/auto_generated_lens_includes/load_lt_sample_aperture.h"    
  }

  sensor[0] = x; sensor[1] = y; sensor[2] = dx; sensor[3] = dy; sensor[4] = lambda;
  return std::max(0.0, out[4]);

}

/*
// jacobian of polynomial mapping sensor to outer pupil. in[]: sensor point/direction/lambda.
static inline void lens_evaluate_jacobian(const float *in, float *J)
{
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_jacobian.h"
  J[0]  = dx00; J[1]  = dx01; J[2]  = dx02; J[3]  = dx03; J[4]  = dx04;
  J[5]  = dx10; J[6]  = dx11; J[7]  = dx12; J[8]  = dx13; J[9]  = dx14;
  J[10] = dx20; J[11] = dx21; J[12] = dx22; J[13] = dx23; J[14] = dx24;
  J[15] = dx30; J[16] = dx31; J[17] = dx32; J[18] = dx33; J[19] = dx34;
  J[20] = dx40; J[21] = dx41; J[22] = dx42; J[23] = dx43; J[24] = dx44;
}*/

/*
static inline float lens_det_sensor_to_outer_pupil(const float *sensor, const float *out, const float focus)
{
  float J[25];
  lens_evaluate_jacobian(sensor, J);
  // only interested in how the direction density at the sensor changes wrt the vertex area on the output
  float T[25] = {
    1., 0., focus, 0., 0.,
    0., 1., 0., focus, 0.,
    0., 0., 1., 0., 0.,
    0., 0., 0., 1., 0.,
    0., 0., 0., 0., 1.};
  float JT[25] = {0.};
  for(int i=2;i<4;i++) // only interested in 2x2 subblock.
    for(int j=0;j<2;j++)
      for(int k=0;k<4;k++)
        JT[i+5*j] += J[k + 5*j] * T[i + 5*k];
  const float det = JT[2] * JT[5+3] - JT[3] * JT[5+2];

  // convert from projected disk to point on hemi-sphere
  const float R = lens_outer_pupil_curvature_radius;
  const float deto = std::sqrt(R*R-out[0]*out[0]-out[1]*out[1])/R;
  // there are two spatial components which need conversion to dm:
  const float dm2mm = 100.0f;
  return fabsf(det * deto) / (dm2mm*dm2mm);
}*/

/*
static inline void lens_evaluate_aperture_jacobian(const float *in, float *J)
{
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_aperture_jacobian.h"
  J[0]  = dx00; J[1]  = dx01; J[2]  = dx02; J[3]  = dx03; J[4]  = dx04;
  J[5]  = dx10; J[6]  = dx11; J[7]  = dx12; J[8]  = dx13; J[9]  = dx14;
  J[10] = dx20; J[11] = dx21; J[12] = dx22; J[13] = dx23; J[14] = dx24;
  J[15] = dx30; J[16] = dx31; J[17] = dx32; J[18] = dx33; J[19] = dx34;
  J[20] = dx40; J[21] = dx41; J[22] = dx42; J[23] = dx43; J[24] = dx44;
}*/


// maps points on the unit square onto the unit disk uniformly
inline void concentric_disk_sample(const double ox, const double oy, Eigen::Vector2d &unit_disk, bool fast_trigo)
{
  double phi, r;

  // switch coordinate space from [0, 1] to [-1, 1]
  double a = 2.0*ox - 1.0;
  double b = 2.0*oy - 1.0;

  if ((a*a) > (b*b)){
    r = a;
    phi = (0.78539816339) * (b/a);
  }
  else {
    r = b;
    phi = (M_PI/2.0) - (0.78539816339) * (a/b);
  }

  if (!fast_trigo){
    unit_disk(0) = r * std::cos(phi);
    unit_disk(1) = r * std::sin(phi);
  } else {
    unit_disk(0) = r * fast_cos(phi);
    unit_disk(1) = r * fast_sin(phi);
  }
}



/*
static inline int lens_clip_aperture(const float x, const float y, const float radius, const int blades)
{ 
  // early out
  if(x*x + y*y > radius*radius) return 0;
  float xx = radius; 
  float yy = 0.0f;
  for(int b=1;b<blades+1;b++)
  {   
  float tmpx, tmpy;
  common_sincosf(2.0f*(float)M_PI/blades * b, &tmpy, &tmpx);
  tmpx *= radius;
  tmpy *= radius;
  const float normalx = xx + tmpx;
  const float normaly = yy + tmpy;
  float dot0 = (normalx)*(x-xx) + (normaly)*(y-yy);
  if(dot0 > 0.0f) return 0;
  xx = tmpx;
  yy = tmpy;
  }
  return 1;
}*/


/*
static inline float lens_det_aperture_to_sensor(const float *sensor, const float focus)
{ 
  float J[25];
  lens_evaluate_aperture_jacobian(sensor, J);
  // only interested in how the directional density at the sensor changes wrt the vertex area (spatial) at the aperture
  float T[25] = {
  1., 0., focus, 0., 0.,
  0., 1., 0., focus, 0.,
  0., 0., 1., 0., 0.,
  0., 0., 0., 1., 0.,
  0., 0., 0., 0., 1.};
  float JT[25] = {0.};
  for(int i=2;i<4;i++) // only interested in 2x2 subblock.
  for(int j=0;j<2;j++)
    for(int k=0;k<4;k++)
    JT[i+5*j] += J[k + 5*j] * T[i + 5*k];
  const float det = fabsf(JT[2] * JT[5+3] - JT[3] * JT[5+2]);
  // there are two spatial components which need conversion to dm:
  const float dm2mm = 100.0f;
  return dm2mm*dm2mm/det;
}*/

/*
static inline float lens_aperture_area(const float radius, const int blades)
{
  const float tri = .5f*radius * radius * sinf(2.0f*AI_PI/(float)blades);
  return blades * tri;
}*/








// returns sensor offset in mm
// traces rays backwards through the lens
inline double camera_set_focus(double dist, Camera *camera)
{
  const Eigen::Vector3d target(0, 0, dist);
  Eigen::VectorXd sensor(5); sensor.setZero();
  Eigen::VectorXd out(5); out.setZero();
  sensor(4) = camera->lambda;
  double offset = 0.0;
  int count = 0;
  const double scale_samples = 0.1;
  Eigen::Vector2d aperture(0,0);

  const int S = 4;

  // trace a couple of adjoint rays from there to the sensor and
  // see where we need to put the sensor plane.
  for(int s=1; s<=S; s++){
    for(int k=0; k<2; k++){
      
      // reset aperture
      aperture.setZero();

      aperture[k] = camera->lens_aperture_housing_radius * (s/(S+1.0) * scale_samples); // (1to4)/(4+1) = (.2, .4, .6, .8) * scale_samples

      lens_lt_sample_aperture(target, aperture, sensor, out, camera->lambda, camera);

      if(sensor(2+k) > 0){
        offset += sensor(k)/sensor(2+k);
        printf("\t[LENTIL] raytraced sensor shift at aperture[%f, %f]: %f", aperture(0), aperture(1), sensor(k)/sensor(2+k));
        count ++;
      }
    }
  }

  // average guesses
  offset /= count; 
  
  // the focus plane/sensor offset:
  // negative because of reverse direction
  if(offset == offset){ // check NaN cases
    const double limit = 45.0;
    if(offset > limit){
      printf("[LENTIL] sensor offset bigger than maxlimit: %f > %f", offset, limit);
      return limit;
    } else if(offset < -limit){
      printf("[LENTIL] sensor offset smaller than minlimit: %f < %f", offset, -limit);
      return -limit;
    } else {
      return offset; // in mm
    }
  } else {
    return 0.0;
  }

}



// returns sensor offset in mm
inline double camera_set_focus_infinity(Camera *camera)
{
	double parallel_ray_height = camera->lens_aperture_housing_radius * 0.1;
  const Eigen::Vector3d target(0.0, parallel_ray_height, AI_BIG);
  Eigen::VectorXd sensor(5); sensor.setZero();
  Eigen::VectorXd out(5); out.setZero();
  sensor[4] = camera->lambda;
  double offset = 0.0;
  int count = 0;

  // just point through center of aperture
  Eigen::Vector2d aperture(0, parallel_ray_height);

  const int S = 4;

  // trace a couple of adjoint rays from there to the sensor and
  // see where we need to put the sensor plane.
  for(int s=1; s<=S; s++){
    for(int k=0; k<2; k++){
      
      // reset aperture
      aperture(0) = 0.0f;
      aperture(1) = parallel_ray_height;

      lens_lt_sample_aperture(target, aperture, sensor, out, camera->lambda, camera);

      if(sensor(2+k) > 0){
        offset += sensor(k)/sensor(2+k);
        count ++;
      }
    }
  }

  offset /= count; 
  
  // check NaN cases
  if(offset == offset){
    return offset;
  } else {return 0.0;}
}


inline std::vector<double> logarithmic_values()
{
  double min = 0.0;
  double max = 45.0;
  double exponent = 2.0; // sharpness
  std::vector<double> log;

  for(double i = -1.0; i <= 1.0; i += 0.0001) {
    log.push_back((i < 0 ? -1 : 1) * std::pow(i, exponent) * (max - min) + min);
  }

  return log;
}


// line plane intersection with fixed intersection at y = 0
// used for finding the focal length and sensor shift
inline Eigen::Vector3d line_plane_intersection(Eigen::Vector3d rayOrigin, Eigen::Vector3d rayDirection)
{
  Eigen::Vector3d coord(100.0, 0.0, 100.0);
  Eigen::Vector3d planeNormal(0.0, 1.0, 0.0);
  rayDirection.normalize();
  coord.normalize();
  return rayOrigin + (rayDirection * (coord.dot(planeNormal) - planeNormal.dot(rayOrigin)) / planeNormal.dot(rayDirection));
}


inline double camera_get_y0_intersection_distance(double sensor_shift, double intersection_distance, Camera *camera)
{
  Eigen::VectorXd sensor(5); sensor.setZero();
  Eigen::VectorXd aperture(5); aperture.setZero();
  Eigen::VectorXd out(5); out.setZero();
  sensor(4) = camera->lambda;
  aperture(1) = camera->lens_aperture_housing_radius * 0.25;

  lens_pt_sample_aperture(sensor, aperture, sensor_shift, camera);

  sensor(0) += sensor(2) * sensor_shift;
	sensor(1) += sensor(3) * sensor_shift;

	double transmittance = lens_evaluate(sensor, out, camera);

	// convert from sphere/sphere space to camera space
  Eigen::Vector2d outpos(out(0), out(1));
  Eigen::Vector2d outdir(out(2), out(3));
	Eigen::Vector3d camera_space_pos(0,0,0);
	Eigen::Vector3d camera_space_omega(0,0,0);
  if (camera->lens_outer_pupil_geometry == "cyl-y") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius, true);
	else if (camera->lens_outer_pupil_geometry == "cyl-x") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius, false);
  else sphereToCs(outpos, outdir, camera_space_pos, camera_space_omega, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius);
  
  return line_plane_intersection(camera_space_pos, camera_space_omega)(2);
}


// focal_distance is in mm
inline double logarithmic_focus_search(const double focal_distance, Camera *camera){
  double closest_distance = 999999999.0;
  double best_sensor_shift = 0.0;
  for (double sensorshift : logarithmic_values()){
  	double intersection_distance = 0.0;
    intersection_distance = camera_get_y0_intersection_distance(sensorshift, intersection_distance, camera);
    double new_distance = focal_distance - intersection_distance;

    if (new_distance < closest_distance && new_distance > 0.0){
      closest_distance = new_distance;
      best_sensor_shift = sensorshift;
    }
  }

  return best_sensor_shift;
}



inline bool trace_ray_focus_check(double sensor_shift, double &test_focus_distance, Camera *camera)
{
  Eigen::VectorXd sensor(5); sensor.setZero();
  Eigen::VectorXd aperture(5); aperture.setZero();
  Eigen::VectorXd out(5); out.setZero();
  sensor(4) = camera->lambda;
  aperture(1) = camera->lens_aperture_housing_radius * 0.25;

	lens_pt_sample_aperture(sensor, aperture, sensor_shift, camera);

  // move to beginning of polynomial
	sensor(0) += sensor(2) * sensor_shift;
	sensor(1) += sensor(3) * sensor_shift;

	// propagate ray from sensor to outer lens element
  double transmittance = lens_evaluate(sensor, out, camera);
  if(transmittance <= 0.0) return false;

  // crop out by outgoing pupil
  if( out(0)*out(0) + out(1)*out(1) > camera->lens_outer_pupil_radius*camera->lens_outer_pupil_radius){
    return false;
  }

  // crop at inward facing pupil
  const double px = sensor(0) + sensor(2) * camera->lens_back_focal_length;
  const double py = sensor(1) + sensor(3) * camera->lens_back_focal_length;
  if (px*px + py*py > camera->lens_inner_pupil_radius*camera->lens_inner_pupil_radius){
    return false;
  }

	// convert from sphere/sphere space to camera space
  Eigen::Vector2d outpos(out(0), out(1));
  Eigen::Vector2d outdir(out(2), out(3));
	Eigen::Vector3d camera_space_pos(0,0,0);
	Eigen::Vector3d camera_space_omega(0,0,0);
  if (camera->lens_outer_pupil_geometry == "cyl-y") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius, true);
	else if (camera->lens_outer_pupil_geometry == "cyl-x") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius, false);
  else sphereToCs(outpos, outdir, camera_space_pos, camera_space_omega, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius);

  test_focus_distance = line_plane_intersection(camera_space_pos, camera_space_omega)(2);
  return true;
}



inline float calculate_distance_vec2(Eigen::Vector2d a, Eigen::Vector2d b) { 
    return std::sqrt(std::pow(b[0] - a[0], 2) +  std::pow(b[1] - a[1], 2));
}

inline Eigen::Vector3d chromatic_abberration_empirical(Eigen::Vector2d pos, float distance_mult, Eigen::Vector2d &lens, float apertureradius) {
  float distance_to_center = calculate_distance_vec2(Eigen::Vector2d(0.0, 0.0), pos);
  int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));

  Eigen::Vector2d aperture_0_center(0.0, 0.0);
  Eigen::Vector2d aperture_1_center(- pos * distance_to_center * distance_mult);
  Eigen::Vector2d aperture_2_center(pos * distance_to_center * distance_mult);

  Eigen::Vector3d weight(1.0, 1.0, 1.0);

  if (random_aperture == 0) {
      if (std::pow(lens(0)-aperture_1_center(0), 2) + std::pow(lens(1) - aperture_1_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(0) = 0.0;
      }
      if (std::pow(lens(0)-aperture_0_center(0), 2) + std::pow(lens(1) - aperture_0_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(2) = 0.0;
      }
      if (std::pow(lens(0)-aperture_2_center(0), 2) + std::pow(lens(1) - aperture_2_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(1) = 0.0;
      }
  } else if (random_aperture == 1) {
      lens += aperture_1_center;
      if (std::pow(lens(0)-aperture_1_center(0), 2) + std::pow(lens(1) - aperture_1_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(0) = 0.0;
      }
      if (std::pow(lens(0)-aperture_0_center(0), 2) + std::pow(lens(1) - aperture_0_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(2) = 0.0;
      }
      if (std::pow(lens(0)-aperture_2_center(0), 2) + std::pow(lens(1) - aperture_2_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(1) = 0.0;
      }
  } else if (random_aperture == 2) {
      lens += aperture_2_center;
      if (std::pow(lens(0)-aperture_1_center(0), 2) + std::pow(lens(1) - aperture_1_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(0) = 0.0;
      }
      if (std::pow(lens(0)-aperture_0_center(0), 2) + std::pow(lens(1) - aperture_0_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(2) = 0.0;
      }
      if (std::pow(lens(0)-aperture_2_center(0), 2) + std::pow(lens(1) - aperture_2_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(1) = 0.0;
      }
  }

  return weight;
}
   


inline void trace_ray(bool original_ray, int &tries, 
                      const double input_sx, const double input_sy, 
                      const double input_lensx, const double input_lensy, 
                      double &r1, double &r2, 
                      Eigen::Vector3d &weight, Eigen::Vector3d &origin, Eigen::Vector3d &direction, 
                      Camera *camera)
{

  tries = 0;
  bool ray_succes = false;

  Eigen::VectorXd sensor(5); sensor.setZero();
  Eigen::VectorXd aperture(5); aperture.setZero();
  Eigen::VectorXd out(5); out.setZero();

  while(!ray_succes && tries <= camera->vignetting_retries){

  	// set sensor position coords
	  sensor(0) = input_sx * (camera->sensor_width * 0.5);
	  sensor(1) = input_sy * (camera->sensor_width * 0.5);
  	sensor(2) = sensor(3) = 0.0;
	  sensor(4) = camera->lambda;

    aperture.setZero();
    out.setZero();


	  if (!camera->enable_dof) aperture(0) = aperture(1) = 0.0; // no dof, all rays through single aperture point
	  
    Eigen::Vector2d unit_disk(0.0, 0.0);
    if (tries == 0 && camera->enable_dof) {
      if (camera->bokeh_enable_image) {
          camera->image.bokehSample(input_lensx, input_lensy, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
      } else if (camera->bokeh_aperture_blades < 2) {
        concentric_disk_sample(input_lensx, input_lensy, unit_disk, false);
      } else {
        lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), input_lensx, input_lensy, 1.0, camera->bokeh_aperture_blades);
      }
    } else if (tries > 0 && camera->enable_dof){
        r1 = xor128() / 4294967296.0;
        r2 = xor128() / 4294967296.0;
        
        if (camera->bokeh_enable_image) {
          camera->image.bokehSample(r1, r2, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
        } else if (camera->bokeh_aperture_blades < 2) {
          concentric_disk_sample(r1, r2, unit_disk, true);
        } else {
          lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), r1, r2, 1.0, camera->bokeh_aperture_blades);
        }
		  }

      aperture(0) = unit_disk(0) * camera->aperture_radius;
      aperture(1) = unit_disk(1) * camera->aperture_radius;
	  
	  


    // if (camera->empirical_ca_dist > 0.0) {
    //   Eigen::Vector2d sensor_pos(sensor[0], sensor[1]);
    //   Eigen::Vector2d aperture_pos(aperture[0], aperture[1]);
    //   weight = chromatic_abberration_empirical(sensor_pos, camera->empirical_ca_dist, aperture_pos, camera->aperture_radius);
    //   aperture(0) = aperture_pos(0);
    //   aperture(1) = aperture_pos(1);
    // }
    

	  if (camera->enable_dof) {
	  	// aperture sampling, to make sure ray is able to propagate through whole lens system
	  	lens_pt_sample_aperture(sensor, aperture, camera->sensor_shift, camera);
	  }
	  

	  // move to beginning of polynomial
		sensor(0) += sensor(2) * camera->sensor_shift;
		sensor(1) += sensor(3) * camera->sensor_shift;


		// propagate ray from sensor to outer lens element
	  double transmittance = lens_evaluate(sensor, out, camera);
		if(transmittance <= 0.0) {
			++tries;
			continue;
		}


		// crop out by outgoing pupil
		if( out(0)*out(0) + out(1)*out(1) > camera->lens_outer_pupil_radius*camera->lens_outer_pupil_radius){
			++tries;
			continue;
		}


		// crop at inward facing pupil
		const double px = sensor(0) + sensor(2) * camera->lens_back_focal_length;
		const double py = sensor(1) + sensor(3) * camera->lens_back_focal_length; //(note that lens_back_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
		if (px*px + py*py > camera->lens_inner_pupil_radius*camera->lens_inner_pupil_radius) {
			++tries;
			continue;
		}
		
		ray_succes = true;
	}

	if (ray_succes == false) weight.setZero();


	// convert from sphere/sphere space to camera space
  Eigen::Vector2d outpos(out[0], out[1]);
  Eigen::Vector2d outdir(out[2], out[3]);
  Eigen::Vector3d cs_origin(0,0,0);
  Eigen::Vector3d cs_direction(0,0,0);
  if (camera->lens_outer_pupil_geometry == "cyl-y") cylinderToCs(outpos, outdir, cs_origin, cs_direction, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius, true);
	else if (camera->lens_outer_pupil_geometry == "cyl-x") cylinderToCs(outpos, outdir, cs_origin, cs_direction, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius, false);
  else sphereToCs(outpos, outdir, cs_origin, cs_direction, -camera->lens_outer_pupil_curvature_radius, camera->lens_outer_pupil_curvature_radius);
  
  origin = cs_origin;
  direction = cs_direction;

  //printf("[%f,%f,%f],", origin[0], origin[1], origin[2]);


  switch (camera->unitModel){
    case mm:
    {
      origin *= -1.0; // reverse rays and convert to cm (from mm)
      direction *= -1.0; //reverse rays and convert to cm (from mm)
    } break;
    case cm:
    { 
      origin *= -0.1; // reverse rays and convert to cm (from mm)
      direction *= -0.1; //reverse rays and convert to cm (from mm)
    } break;
    case dm:
    {
      origin *= -0.01; // reverse rays and convert to cm (from mm)
      direction *= -0.01; //reverse rays and convert to cm (from mm)
    } break;
    case m:
    {
      origin *= -0.001; // reverse rays and convert to cm (from mm)
      direction *= -0.001; //reverse rays and convert to cm (from mm)
    }
  }

  direction.normalize();

  // Nan bailout
  if (origin(0) != origin(0) || origin(1) != origin(1) || origin(2) != origin(2) || 
    direction(0) != direction(0) || direction(1) != direction(1) || direction(2) != direction(2))
  {
    weight.setZero();
  }

}


// given camera space scene point, return point on sensor
inline bool trace_backwards(Eigen::Vector3d target, 
                            const double aperture_radius, 
                            const double lambda,
                            Eigen::Vector2d &sensor_position, 
                            const double sensor_shift, 
                            Camera *camera, 
                            const int px, 
                            const int py,
                            const int total_samples_taken,
                            AtMatrix cam_to_world,
                            AtVector sample_pos_ws,
                            AtShaderGlobals *sg)
{
  int tries = 0;
  bool ray_succes = false;

  // initialize 5d light fields
  Eigen::VectorXd sensor(5); sensor << 0,0,0,0, lambda;
  Eigen::VectorXd out(5); out << 0,0,0,0, lambda;//out.setZero();
  Eigen::Vector2d aperture(0,0);
  
  while(ray_succes == false && tries <= camera->vignetting_retries){

    Eigen::Vector2d unit_disk(0.0, 0.0);

    if (!camera->enable_dof) aperture(0) = aperture(1) = 0.0; // no dof, all rays through single aperture point
	  else if (camera->enable_dof && camera->bokeh_aperture_blades <= 2) {
      unsigned int seed = tea<8>(px*py+px, total_samples_taken+tries);

      if (camera->bokeh_enable_image) camera->image.bokehSample(rng(seed), rng(seed), unit_disk, rng(seed), rng(seed));
      else concentric_disk_sample(rng(seed), rng(seed), unit_disk, true);

      aperture(0) = unit_disk(0) * camera->aperture_radius;
      aperture(1) = unit_disk(1) * camera->aperture_radius;
	  } 
	  else if (camera->enable_dof && camera->bokeh_aperture_blades > 2) {
      unsigned int seed = tea<8>(px*py+px, total_samples_taken+tries);
      lens_sample_triangular_aperture(aperture(0), aperture(1), rng(seed), rng(seed), camera->aperture_radius, camera->bokeh_aperture_blades);
    }


    // raytrace for scene/geometrical occlusions along the ray
    AtVector lens_correct_scaled = AtVector(-aperture(0)*0.1, -aperture(1)*0.1, 0.0);
    switch (camera->unitModel){
      case mm: { lens_correct_scaled /= 0.1; } break;
      case cm: { lens_correct_scaled /= 1.0; } break;
      case dm: { lens_correct_scaled /= 10.0;} break;
      case m:  { lens_correct_scaled /= 100.0;}
    }
    AtVector cam_pos_ws = AiM4PointByMatrixMult(cam_to_world, lens_correct_scaled);
    AtVector ws_direction = cam_pos_ws - sample_pos_ws;
    AtRay ray = AiMakeRay(AI_RAY_SHADOW, sample_pos_ws, &ws_direction, AI_BIG, sg);
    if (AiTraceProbe(ray, sg)){
      ++tries;
      continue;
    }

    sensor(0) = sensor(1) = 0.0;

    float transmittance = lens_lt_sample_aperture(target, aperture, sensor, out, lambda, camera);
    if(transmittance <= 0) {
      ++tries;
      continue;
    }

    // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
    const double px = sensor(0) + sensor(2) * camera->lens_back_focal_length;
    const double py = sensor(1) + sensor(3) * camera->lens_back_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
    if (px*px + py*py > camera->lens_inner_pupil_radius*camera->lens_inner_pupil_radius) {
      ++tries;
      continue;
    }

    ray_succes = true;
  }

  // need to account for the ray_success==false case
  if (!ray_succes) return false;

  // shift sensor
  sensor(0) += sensor(2) * -sensor_shift;
  sensor(1) += sensor(3) * -sensor_shift;

  sensor_position(0) = sensor(0);
  sensor_position(1) = sensor(1);

  return true;
}


// note that this is all with an unshifted sensor
inline void trace_backwards_for_fstop(Camera *camera, const double fstop_target, double &calculated_fstop, double &calculated_aperture_radius)
{
  const int maxrays = 1000;
  double best_valid_fstop = 0.0;
  double best_valid_aperture_radius = 0.0;

  for (int i = 1; i < maxrays; i++)
  {
    const double parallel_ray_height = (static_cast<double>(i)/static_cast<double>(maxrays)) * camera->lens_outer_pupil_radius;
    const Eigen::Vector3d target(0, parallel_ray_height, AI_BIG);
    Eigen::VectorXd sensor(5); sensor << 0,0,0,0, camera->lambda;
    Eigen::VectorXd out(5); out.setZero();

    // just point through center of aperture
    Eigen::Vector2d aperture(0.01, parallel_ray_height);

    // if (!lens_lt_sample_aperture(target, aperture, sensor, out, camera->lambda, camera)) continue;
    if(lens_lt_sample_aperture(target, aperture, sensor, out, camera->lambda, camera) <= 0.0) continue;

    // crop at inner pupil
    const double px = sensor(0) + (sensor(2) * camera->lens_back_focal_length);
    const double py = sensor(1) + (sensor(3) * camera->lens_back_focal_length);
    if (px*px + py*py > camera->lens_inner_pupil_radius*camera->lens_inner_pupil_radius) continue;

    // somehow need to get last vertex positiondata.. don't think what i currently have is correct
    Eigen::Vector3d out_cs_pos(0,0,0);
    Eigen::Vector3d out_cs_dir(0,0,0);
    Eigen::Vector2d outpos(out(0), out(1));
    Eigen::Vector2d outdir(out(2), out(3)); 
    if (camera->lens_inner_pupil_geometry == "cyl-y") {
      cylinderToCs(outpos, outdir, out_cs_pos, out_cs_dir, - camera->lens_inner_pupil_curvature_radius + camera->lens_back_focal_length, camera->lens_inner_pupil_curvature_radius, true);
    }
    else if (camera->lens_inner_pupil_geometry == "cyl-x") {
      cylinderToCs(outpos, outdir, out_cs_pos, out_cs_dir, - camera->lens_inner_pupil_curvature_radius + camera->lens_back_focal_length, camera->lens_inner_pupil_curvature_radius, false);
    }
    else sphereToCs(outpos, outdir, out_cs_pos, out_cs_dir, - camera->lens_inner_pupil_curvature_radius + camera->lens_back_focal_length, camera->lens_inner_pupil_curvature_radius);

    const double theta = std::atan(out_cs_pos(1) / out_cs_pos(2));
    const double fstop = 1.0 / (std::sin(theta)* 2.0);

    if (fstop < fstop_target) {
      calculated_fstop = best_valid_fstop;
      calculated_aperture_radius = best_valid_aperture_radius;
      return;
    } else {
      best_valid_fstop = fstop;
      best_valid_aperture_radius = parallel_ray_height;
    }
  }

  calculated_fstop = best_valid_fstop;
  calculated_aperture_radius = best_valid_aperture_radius;
}
