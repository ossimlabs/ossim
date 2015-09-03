//---------------------------------------------------------------------
// $Id: bilinear_projection_template.geom 7662 2005-06-15 16:36:23Z dburken $
//
// Description:
// This is a keyword list with minimum set of key words to make an
// ossimBilinearProjection.
//
// NOTE:
// The dpt's are in image space(x, y)
//
// The gpt's are in decimal degrees (latitude, longitude, height, datum)
//
// gpt0 should correspond to dpt0.
//
// It is best to have at least the four corners of the image.
//---------------------------------------------------------------------
dpt0:  ( 0.0, 0.0)
dpt1:  ( 2047.0, 0.0 )
dpt2:  ( 2047.0, 2047.0 )
dpt3:  ( 0.0, 2027.0 )
gpt0:  ( 37.0, -117.0, 0.0, WGE )
gpt1:  ( 37.0, -116.0, 0.0, WGE )
gpt2:  ( 36.0, -116.0, 0.0, WGE )
gpt3:  ( 36.0, -117.0, 0.0, WGE )
type:  ossimBilinearProjection
