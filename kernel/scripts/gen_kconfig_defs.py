import kconfiglib # pip install kconfiglib
import os

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    kconfig_path = os.path.join(script_dir, '../Kconfig')
    def_path = os.path.join(script_dir, '../defconfig')

    # Load the main Kconfig file
    kconf = kconfiglib.Kconfig(kconfig_path)
    
    # Load an existing configuration file
    kconf.load_config(def_path)
    
    # use standard output redirection (>) to save the output config to C header
    print("/* Auto-generated configuration header */\n")
    print("#ifndef GEN_KCONFIG_DEFS_H")
    print("#define GEN_KCONFIG_DEFS_H\n")
    
    for sym in kconf.unique_defined_syms:
        if sym.str_value == "n":
            print(f"#undef {sym.name}\n")
        elif sym.type in (kconfiglib.BOOL, kconfiglib.TRISTATE):
            if sym.str_value == "y":
                print(f"#define {sym.name} 1\n")
        elif sym.type == kconfiglib.INT or sym.type == kconfiglib.STRING:
            print(f"#define {sym.name} {sym.str_value}\n")
        elif sym.type == kconfiglib.HEX:
            print(f"#define {sym.name} 0x{sym.str_value}\n")

    print("#endif /* #ifndef GEN_KCONFIG_DEFS_H*/")

if __name__ == '__main__':
    main()
