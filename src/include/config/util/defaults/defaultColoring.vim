syntax match KessPrint /print/
syntax match KessStringLit /\v(['"])%(\1@![^\\]|\\.)*\1/
syntax match KessComment /!!.*/
syntax match KessVar /\(?\([a-zA-Z]\|_\)\w\+\|^?[A-Za-z]\{1}\)/
syntax match KessInt /[0-9]\+/
syntax match KessHex /0\(x\|X\)[0-9A-Fa-f]/
syntax match KessAssignment /=/


highlight KessPrint ctermfg=220 guifg=#ffd700
" highlight KessStringLit ctermfg=99 guifg=#875fff
highlight KessStringLit ctermfg=85 guifg=#5fffaf
highlight KessComment ctermfg=245 guifg=#8a8a8a
highlight KessVar ctermfg=79 guifg=#5fd7af
highlight KessInt ctermfg=205 guifg=#ff5faf
highlight KessHex ctermfg=205 guifg=#ff5faf
highlight KessAssignment ctermfg=223 guifg=#ffd7af
