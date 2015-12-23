The GenSecImage tool is used to generate secured images.

ReadMe.txt           - This file.
gensecimage.py       - Main GenSecImage python script.
mbn_tools.py         - Library for image processing.
gensecimageconfig.py - Script to use meta build to generate GenSecImage configuration files.
qpsa                 - Directory containing Qualcomm Platform Signing Application (QPSA)
                       scripts and resources.
ssd                  - Directory containing Qualcomm Secured Software Download (SSD)
                       tool and resources.
resources            - Directory containing GenSecImage resources

------------------------------------------------------------------------------
 Generating basic configuration
------------------------------------------------------------------------------
If resources/<target>_gensecimage.cfg exists, it can be used directly with gensecimage.
Edit file_name and output_dir if necessary. Go to the next section.

If resources/<target>_gensecimage.cfg.template file exists, follow the steps in this section.

Use the configuration template files in resources to create a basic configuration file.
For example:

1. Copy resources/<target>_gensecimage.cfg.template to
   resources/<target>_gensecimage.cfg.
2. In each section in <target>_gensecimage.cfg, do the following:
    (a) Edit file_name to point to the correct location of the unsigned image.
        This can be a relative path, an absolute path, or a network location.
    (b) Make changes to output_dir, if necessary.



------------------------------------------------------------------------------
 Generating postsign configuration
------------------------------------------------------------------------------
Postsign configuration is optional. If resources/<target>_postsigncmd.cfg.template
file exists, follow the steps in this section.

Use the configuration template files in resources to create a postsign configuration file.
For example:
1. Copy resources/<target>_postsigncmd.cfg.template to resources/<target>_postsigncmd.cfg.
2. In <target specific>_postsigncmd.cfg, do the following:
    (a) Specify meta_build_dir to point to the <meta build> location if it presents.
        This is used to call the pil-splitter tool from the meta build location.
        If the pil-splitter tool exists in a different directory, also update the
        pil_splitter_dir parameter directly.
    (b) Specify build_dir1 to point to the <modem build> directory if it presents.
        This is used to call the mba_elf_builder tool from the modem build location.
        If the mba_elf_builder tool exists in a different directory, also update the
        mba_elf_builder_dir parameter directly.

------------------------------------------------------------------------------
 Generating configurations from meta build
------------------------------------------------------------------------------

Alternatively, gensecimageconfig.py can be used to generate the configurations
automatically by specifying a meta build location.
1. Run the following command. The <meta build> location can be a relative path,
   an absolute path, or a network location.

   python gensecimageconfig.py -metabuild=<meta build>
                --basictemplate=resources/<target>_gensecimage.cfg.template
                --postsigntemplate=resources/<target>_postsigncmd.cfg.template

   The above usage generates resources/<target>_gensecimage.cfg and
   resources/<target>_postsigncmd.cfg

2. After running the gensecimageconfig command, open <target>_gensecimage.cfg.
    (a) Ensure that all file_name sections are specified.
    (b) Make changes to output_dir, if necessary.

------------------------------------------------------------------------------
 Running gensecimage
------------------------------------------------------------------------------
1. Once configuration files are ready, run the following command to sign images:

   python gensecimage.py -stage=<desired stage> --section=<desired section>
                         --config=resources/<target>_gensecimage.cfg

or

   python gensecimage.py -stage=<desired stage> --section=<desired section>
                         --config=resources/<target>_gensecimage.cfg
                         --signconfig=resources/<target>_signingattr_qpsa.cfg
                         --postsignconfig=resources/<target>_postsigncmd.cfg

2. Run the following to see usage for the tool:
   python gensecimage.py

--------------------------------------------------------------------------------
Disclaimer
--------------------------------------------------------------------------------
The licensee takes responsibility for the use of the information and tools in
the gensecimage and QPSA. The licensee takes responsibility for what code they authorize.
Should the licensee sign malware, poorly behaving or poorly developed code,
QC takes no responsibility for such code.

All the non-commercial root certificates packaged in QPSA are intended only for pre-commercial
device development and is NOT suitable for inclusion in commercial devices.
Qualcomm makes no effort to safeguard the security of the private key of
the non-commmercial root certificates or the private key of
the non-commercial CA certificates or the non-commercial CA certificates
issued under the non-commerical root certficiates each of which are intended for distribution
to multiple unknown parties allowing such parties to generate signatures that will allow applications
to execute on devices which embed the non-commercial root certificate.
