//---------------------------------------------------------------------
// $Id: utm_projection_template.geom 22468 2013-10-27 15:45:39Z dburken $
//
// Description:
// This is a keyword list with minimum set of key words to make an
// ossimUtmProjection.
//---------------------------------------------------------------------


//-----------------
// Short version with code, tie and scale:
// For codes reference: http://spatialreference.org/
//-----------------
pcs_code:  32616
pixel_scale_units:  meters
pixel_scale_xy:  ( 5.0, 5.0 )
pixel_type:  point
tie_point_units:  meters
tie_point_xy:  ( 138425.0, 4738765.0 ) 


//-----------------
// Long version:
//-----------------
central_meridian:  -87.000000000000000
datum:  WGE
ellipse_code:  WE
ellipse_name:  WGS 84
false_easting_northing:  ( 500000.000000000000000, 0.000000000000000 )
false_easting_northing_units:  meters
hemisphere:  N
major_axis:  6378137.000000000000000
minor_axis:  6356752.314199999906123
origin_latitude:  0.000000000000000
pixel_scale_units:  meters
pixel_scale_xy:  ( 5.0, 5.0 )

// point(tie center of pixel) or area (tie upper left corner of pixel).
pixel_type:  point

tie_point_units:  meters
tie_point_xy:  ( 138425.0, 4738765.0 )
type:  ossimUtmProjection
zone:  16
