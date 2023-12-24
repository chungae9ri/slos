add_library(
  fs_objs OBJECT
  fs.c
  littlefs/lfs.c
  littlefs/lfs_util.c
  slfs/slfs.c
)

target_include_directories(fs_objs PRIVATE littlefs)
