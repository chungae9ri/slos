#===============================================================================
#
# gensecimageconfig.py
#
# GENERAL DESCRIPTION
#    Contains tools to generate gensecimage config from meta build
#
# Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#-------------------------------------------------------------------------------
#
#  $Header:
#  $DateTime: 2013/09/26 15:43:39 $
#  $Author: yliong $
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who     what, where, why
# --------   ---     ---------------------------------------------------------
#
#===============================================================================
import sys
import ConfigParser
import os
import getopt

CURRENT_VERSION = '1.0.0'

meta_lib_relative_path='/common/tools/meta'
metabuild_dir=''
basictemplate_to_read=''
postsigntemplate_to_read=''
basic_out_cfg=''
postsign_out_cfg=''

gensecimage_dir = '../'
# Set environment variable
gensecimage_dir = os.environ.get('GENSECIMAGE_DIR', failobj='../')
if (gensecimage_dir.endswith('/') is False):
   gensecimage_dir = gensecimage_dir + '/'
os.environ['GENSECIMAGE_DIR'] = gensecimage_dir
GENSECIMAGE_PY = gensecimage_dir + 'gensecimage.py'  

TEMPLATETYPE_BASICCONFIG = 'config'
TEMPLATETYPE_POSTSIGNCONFIG = 'postsignconfig'

META_BUILD_DIR_OPTION = 'meta_build_dir'
BUILD_DIR_NAME_OPTION = 'build_dir_name'
BUILD_DIR_OPTION = 'build_dir'


#----------------------------------------------------------------------------
# invokeGenSecImage
# Invoke GenSecImage tool
#----------------------------------------------------------------------------
def invokeGenSecImage(stage, section, config):  
  if os.path.exists(GENSECIMAGE_PY) is True:
      # Issue GenSecImage command 
      print 'python ' + GENSECIMAGE_PY + ' --stage=' + stage +\
                ' --section=' + section + \
                ' --config=' + config     
      errorNumber = os.system('python ' + GENSECIMAGE_PY + ' --stage=' + stage +\
                ' --section=' + section + \
                ' --config=' + config\
               )      
      print 'Error number = ' , errorNumber
      return errorNumber
  else:
    raise RuntimeError, "GenSecImage script not found"	 

#----------------------------------------------------------------------------
# usage
# Help on how to use this script
#----------------------------------------------------------------------------
def usage ():
  print "========================================================"
  print "-h, --help       prints this help"
  print "-v               prints version"  
  print ""
  print "usage"
  print "gensecimageconfig.py --metabuild=<metabuild location>"
  print "                   --basictemplate=<basic config template>"
  print "                   --postsigntemplate=<postsign config template> "
  print "                   --basicoutput=<basic output config> "
  print "                   --postsignoutput=<postsign output config> " 
  print "   where <metabuild location> is the location of the meta build"
  print "   where <basic config template> is the basic config template to use."
  print "   where <postsign config template> is the postsign config template to use."
  print "   where <basic output config> is the basic output config filename."  
  print "          Optional. Default to <basic config template> without template extension"
  print "   where <postsign output config> is the postsign output config filename."  
  print "          Optional. Default to <postsign config template> without template extension"  
  print ""
  print "e.g: gensecimageconfig.py --metabuild=\\\\fosters\\builds413\\INTEGRATION\\M8974AAAAANLGD101533.1"
  print "          --basictemplate=resources/8974_LA_gensecimage.cfg.template"
  print "          --postsigntemplate=resources/8974_LA_postsigncmd.cfg.template"  
  print ""
  print "The above usage generates resources/8974_LA_gensecimage.cfg"
  print "and resources/8974_LA_postsigncmd.cfg"  
  print ""
  print "========================================================"  
    
def parse_args_and_validate(args):
    global metabuild_dir
    global basictemplate_to_read
    global postsigntemplate_to_read
    global basic_out_cfg
    global postsign_out_cfg
    global type

    metabuild_dir = ''
    basictemplate_to_read = '' 
    postsigntemplate_to_read = ''
    basic_out_cfg = ''
    postsign_out_cfg = ''

    try:
      opts, remainder = getopt.getopt(args, "hv", ["help", "metabuild=", "basictemplate=", "postsigntemplate=", \
                                                    "basicoutput=", "postsignoutput="])
    except getopt.GetoptError, err:
      # print help information and exit:
      print str(err) # will print something like "option -a not recognized"
      usage()
      sys.exit(2)
    if len(sys.argv) < 2:
      usage()
      sys.exit(2)
    for opt, arg in opts:
      if opt in ("-h", "--help"):
        usage()            
        sys.exit()
      if opt in ("-v"):
        print "gensecimageconfig " + CURRENT_VERSION           
        sys.exit()          
      elif opt in ("--metabuild"):
        if (metabuild_dir == ''):
          metabuild_dir = arg
          print "Found argument for metabuild %s" % (metabuild_dir)
        else:	  
          print "Can't have multiple metabuild arguments!"	   
      elif opt in ("--basictemplate"):
        if (basictemplate_to_read == ''):          
          basictemplate_to_read= arg          
          print "Found argument for basic template config %s" % (basictemplate_to_read)
        else:    
           print "Can't have multiple template arguments!"
      elif opt in ("--postsigntemplate"):
        if (postsigntemplate_to_read == ''):          
          postsigntemplate_to_read= arg          
          print "Found argument for postsign template config %s" % (postsigntemplate_to_read)
        else:    
           print "Can't have multiple template arguments!"           
      elif opt in ("--basicoutput"):
        if (basic_out_cfg == ''):          
          basic_out_cfg= arg
          print "Found argument for basic output config %s" % (basic_out_cfg)
        else:    
          print "Can't have multiple outputconfig arguments!"
      elif opt in ("--postsignoutput"):
        if (postsign_out_cfg == ''):          
          postsign_out_cfg= arg
          print "Found argument for postsign output config %s" % (postsign_out_cfg)
        else:    
          print "Can't have multiple outputconfig arguments!"          


    # ensure valid arguments are present
    if (metabuild_dir == ''):
      usage()
      raise RuntimeError, "Missing metabuild directory"
    if (basictemplate_to_read == '' and postsigntemplate_to_read == ''):
      usage()
      raise RuntimeError, "Missing both basic and postsign template file"

    if not os.path.exists(metabuild_dir):
       raise RuntimeError, "Metabuild directory does not exist:" + metabuild_dir   
       
    if basictemplate_to_read != '' and not os.path.isfile(basictemplate_to_read):
       raise RuntimeError, "Basic template file does not exist:" + basictemplate_to_read    

    if postsigntemplate_to_read != '' and not os.path.isfile(postsigntemplate_to_read):
       raise RuntimeError, "Postsign template file does not exist:" + postsigntemplate_to_read 

    if (basic_out_cfg == '' and basictemplate_to_read != ''):
       (basic_out_cfg, ext) = os.path.splitext(basictemplate_to_read)
       print "Output basic config filename: %s" % (basic_out_cfg)  

    if (postsign_out_cfg == '' and postsigntemplate_to_read != ''):       
       (postsign_out_cfg, ext) = os.path.splitext(postsigntemplate_to_read)
       print "Output postsign config filename: %s" % (postsign_out_cfg)
    
    metabuild_dir = os.path.abspath(metabuild_dir)
    print metabuild_dir
    
#----------------------------------------------------------------------------
# MAIN SCRIPT BEGIN
#----------------------------------------------------------------------------
def main():
    parse_args_and_validate(sys.argv[1:])
    
    print "gensecimageconfig starts"
    print "gensecimage = " + GENSECIMAGE_PY     
     
    meta_lib_dir = metabuild_dir + meta_lib_relative_path
    sys.path.append(meta_lib_dir)
    import meta_lib as ml
    mi = ml.meta_info()
   
    #import pdb;pdb.set_trace()    
    if (basictemplate_to_read != ''):
        outputfile = open(basic_out_cfg,'w')
        template_file = open(basictemplate_to_read)     
        
        #Read content.xml to get bid
        contents_xml = open(os.path.join(metabuild_dir, "contents.xml"), 'rt')
        contents_xml_data = contents_xml.read()
        
        # Get file path using cmm_file_var in contents.xml
        file_vars = mi.get_file_vars(
                            file_types=['download_file', 'file_ref'],
                            attr='cmm_file_var')
           
        for line in template_file:           
            for var_name in file_vars:                
                if line.find("%" + var_name + "%")>=0:
                    file_list = file_vars[var_name]
                    print "var_name=%s list length = %d" % (var_name, len(file_list))
                    line = line.replace("%" + var_name + "%", file_list[0])
                    print line
                
            idx1 = line.find("%ROOTPATH_")    
            if (idx1 >= 0):
                idx2 = line.find("%", idx1+1)
                root_path_name = line[idx1+10:idx2]
                print "root_path_name = " + root_path_name
                root_path =  mi.get_build_path(root_path_name)
                line = line.replace("%ROOTPATH_" + root_path_name + "%", root_path)
                print line
                
            idx1 = line.find("%METAPATH%")    
            if (idx1 >= 0):
                line = line.replace("%METAPATH%", metabuild_dir)
                print line
                
            idx1 = line.find("%BID_")    
            if (idx1 >= 0):
                idx2 = line.find("%", idx1+1)
                bid_name = line[idx1+5:idx2]
                print "bid_name = " + bid_name
                bid_key = '${%s_bid:' % bid_name
                bid_idx1 =  contents_xml_data.find(bid_key)
                if (bid_idx1 >=0 ):
                    bid_idx2 = contents_xml_data.find("}", bid_idx1+1)
                    bid = contents_xml_data[bid_idx1+len(bid_key):bid_idx2]
                    print "bid = " + bid
                    line = line.replace("%BID_" + bid_name + "%", bid)
                    print line
                
            outputfile.write(line)                
              
        outputfile.close()
        template_file.close()
        contents_xml.close()        
        
    if (postsigntemplate_to_read != ''):
        config = ConfigParser.SafeConfigParser()
        config.read(postsigntemplate_to_read)  
    
        default_config_dict = config.defaults()
        if META_BUILD_DIR_OPTION in default_config_dict.keys(): 
            config.set('DEFAULT', META_BUILD_DIR_OPTION, metabuild_dir)
        
        i = 1
        while True:
            build_dir_name = BUILD_DIR_NAME_OPTION + str(i)
            if build_dir_name in default_config_dict.keys():
                build_path = mi.get_build_path(default_config_dict[build_dir_name])
                print 'Set ' + default_config_dict[build_dir_name] + ' build dir: ' + build_path             
                config.set('DEFAULT', BUILD_DIR_OPTION + str(i), build_path)
                i = i + 1
            else:
                break
                
        outputfile = open(postsign_out_cfg, "wb")
        config.write(outputfile)
        outputfile.close()
   
if __name__ == "__main__":
  main()
  
#----------------------------------------------------------------------------
# MAIN SCRIPT END 
#----------------------------------------------------------------------------   