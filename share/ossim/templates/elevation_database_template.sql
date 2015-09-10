# MySQL dump 6.0
#
# Host: localhost    Database: ilelev
#--------------------------------------------------------
# Server version	3.22.25

#
# Table structure for table 'level0'
#
CREATE TABLE level0 (
  elev_id bigint(20) DEFAULT '0' NOT NULL auto_increment,
  ll_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ll_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ul_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ul_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ur_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ur_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lr_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lr_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lat_interval int(11) DEFAULT '0' NOT NULL,
  lon_interval int(11) DEFAULT '0' NOT NULL,
  lat_points int(11) DEFAULT '0' NOT NULL,
  lon_lines int(11) DEFAULT '0' NOT NULL,
  vert_datum varchar(10) DEFAULT '' NOT NULL,
  horiz_datum varchar(10) DEFAULT '' NOT NULL,
  abs_ce int(11) DEFAULT '0' NOT NULL,
  abs_le int(11) DEFAULT '0' NOT NULL,
  rel_ce int(11) DEFAULT '0' NOT NULL,
  rel_le tinyint(4) DEFAULT '0' NOT NULL,
  path varchar(255) DEFAULT '' NOT NULL,
  file varchar(255) DEFAULT '' NOT NULL,
  PRIMARY KEY (elev_id),
  KEY elev_id (elev_id),
  UNIQUE elev_id_2 (elev_id)
);

#
# Dumping data for table 'level0'
#


#
# Table structure for table 'level1'
#
CREATE TABLE level1 (
  elev_id bigint(20) DEFAULT '0' NOT NULL auto_increment,
  ll_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ll_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ul_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ul_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ur_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ur_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lr_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lr_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lat_interval int(11) DEFAULT '0' NOT NULL,
  lon_interval int(11) DEFAULT '0' NOT NULL,
  lat_points int(11) DEFAULT '0' NOT NULL,
  lon_lines int(11) DEFAULT '0' NOT NULL,
  vert_datum varchar(10) DEFAULT '' NOT NULL,
  horiz_datum varchar(10) DEFAULT '' NOT NULL,
  abs_ce int(11) DEFAULT '0' NOT NULL,
  abs_le int(11) DEFAULT '0' NOT NULL,
  rel_ce int(11) DEFAULT '0' NOT NULL,
  rel_le tinyint(4) DEFAULT '0' NOT NULL,
  path varchar(255) DEFAULT '' NOT NULL,
  file varchar(255) DEFAULT '' NOT NULL,
  PRIMARY KEY (elev_id),
  KEY elev_id (elev_id),
  UNIQUE elev_id_2 (elev_id)
);

#
# Dumping data for table 'level1'
#


#
# Table structure for table 'level2'
#
CREATE TABLE level2 (
  elev_id bigint(20) DEFAULT '0' NOT NULL auto_increment,
  ll_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ll_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ul_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ul_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ur_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  ur_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lr_lat double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lr_lon double(16,10) DEFAULT '0.0000000000' NOT NULL,
  lat_interval int(11) DEFAULT '0' NOT NULL,
  lon_interval int(11) DEFAULT '0' NOT NULL,
  lat_points int(11) DEFAULT '0' NOT NULL,
  lon_lines int(11) DEFAULT '0' NOT NULL,
  vert_datum varchar(10) DEFAULT '' NOT NULL,
  horiz_datum varchar(10) DEFAULT '' NOT NULL,
  abs_ce int(11) DEFAULT '0' NOT NULL,
  abs_le int(11) DEFAULT '0' NOT NULL,
  rel_ce int(11) DEFAULT '0' NOT NULL,
  rel_le tinyint(4) DEFAULT '0' NOT NULL,
  path varchar(255) DEFAULT '' NOT NULL,
  file varchar(255) DEFAULT '' NOT NULL,
  PRIMARY KEY (elev_id),
  KEY elev_id (elev_id),
  UNIQUE elev_id_2 (elev_id)
);

#
# Dumping data for table 'level2'
#


