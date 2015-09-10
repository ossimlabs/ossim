//---------------------------------------------------------------------
// $Id: geographic_projection_template.geom 20209 2011-11-04 15:21:37Z dburken $
//
// Description:
// This is a keyword list with minimum set of key words to make an
// ossimEquDistCylProjection (commonly called geographic).
//
// NOTE:
// If decimal_degrees_per_pixel_lat equals decimal_degrees_per_pixel_lon then
// your origin_latitude should be 0.0 or at the equator.  If not, the
// decimal_degrees_per_pixel_lon =
// decimal_degrees_per_pixel_lat * cos(fabs(origin_latitude))
//---------------------------------------------------------------------
central_meridian:  0.0
datum:  WGE
ellipse_code:  WE
ellipse_name:  WGS 84
false_easting_northing:  ( 0.000000000000000, 0.000000000000000 )
false_easting_northing_units:  meters
origin_latitude:  0.0
pixel_scale_units:  degrees
pixel_scale_xy:  ( 0.066666667, 0.066666667 )

// point(tie center of pixel) or area (tie upper left corner of pixel).
pixel_type:  point

tie_point_units:  degrees
tie_point_xy:  ( -91.416731292574454, 42.792911858047304 )

type:  ossimEquDistCylProjection

