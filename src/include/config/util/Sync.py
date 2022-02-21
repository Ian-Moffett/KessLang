import re
import os

if os.getcwd() != "/home/ian1/KessLang/src/include/config/util":
    print("Failed to sync. You must be in \"/home/ian1/KessLang2/config/util\"!")
    exit(1);


with open("../Reserved.h", "r") as reserved:
    contents = reserved.read()
    print_statement = re.findall(r"(?<=IO_PRINT_STATEMENT\s\").*\"", contents)[0].strip('"')
    comment_sym = re.findall(r"(?<=COMMENT_SYM)\s+\".*\"", contents)[0].strip(' ').strip('"').replace('/', "\\/");
    var_prefix = re.findall(r"(?<=VAR_PREFIX)\s+\".*\"", contents)[0].strip(' ').strip('"').replace('/', "\\/");
    deref_op = re.findall(r"(?<=DEREF_OP)\s+\".*\"", contents)[0].strip(' ').strip('"').replace('/', "\\/");

    if ' ' in print_statement or ' ' in comment_sym:
        print("KEYWORD_HEADER FORMAT ERROR!\n\nMake sure there are no spaces, speical character etc.")
        exit(1)

    home_dir = os.popen("echo $HOME").read().strip('\n')

    if not os.path.exists(f"{home_dir}/.config/nvim/"):
        print("nvim config not found!")
        exit(1)

    if not os.path.exists(f"{home_dir}/.config/nvim/syntax/"):
        os.mkdir(f"{home_dir}/.config/nvim/syntax/")

    if not os.path.exists(f"{home_dir}/.config/nvim/syntax/kesslang.vim"):
        os.system(f"touch {home_dir}/.config/nvim/syntax/kesslang.vim")
    

    with open("defaults/defaultColoring.vim", "r") as defaults:
        defaultsrepl = defaults.read().replace("/print/", f"/{print_statement}/")
        defaultsrepl = defaultsrepl.replace("/!!.*/", f"/{comment_sym}.*/")
        defaultsrepl = defaultsrepl.replace(r"KessDeref/\*/", f"KessDeref/\{comment_sym}/")
        var_find = r"/\(^?\([a-zA-Z]\|_\)\w\+\|^?[A-Za-z]\{1}\)/".replace('?', var_prefix);
        defaultsrepl = defaultsrepl.replace("/\(^?\([a-zA-Z]\|_\)\w\+\|^?[A-Za-z]\{1}\)/", var_find);
    
    with open(f"{home_dir}/.config/nvim/syntax/kesslang.vim", "w") as syntaxfile:
        syntaxfile.write(defaultsrepl)

        print("Synced Successfully!")
