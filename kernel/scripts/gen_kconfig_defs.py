import kconfiglib # pip install kconfiglib
import os
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--defconfig', type=str, required=True, default='zynq7000_defconfig')
    args = parser.parse_args()

    defconfig = args.defconfig

    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(os.path.join(script_dir, '..'))

    # Load the main Kconfig file
    kconf = kconfiglib.Kconfig("Kconfig")
    
    # Load an existing configuration file
    ret = kconf.load_config(defconfig)

    kconf.write_config(".config")
    
    # use standard output redirection (>) to save the output config to C header
    print("/* Auto-generated configuration header */\n")
    print("#ifndef GEN_KCONFIG_DEFS_H")
    print("#define GEN_KCONFIG_DEFS_H\n")
    
    for sym in kconf.unique_defined_syms:
        if sym.str_value == "n":
            print(f"#undef {sym.name}")
        elif sym.type in (kconfiglib.BOOL, kconfiglib.TRISTATE):
            if sym.str_value == "y":
                print(f"#define {sym.name} 1")
        elif sym.type == kconfiglib.INT or sym.type == kconfiglib.STRING:
            print(f"#define {sym.name} {sym.str_value}")
        elif sym.type == kconfiglib.HEX:
            print(f"#define {sym.name} 0x{sym.str_value}")

    print("\n")
    print("#endif /* #ifndef GEN_KCONFIG_DEFS_H*/")

if __name__ == '__main__':
    main()
