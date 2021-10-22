let &makeprg='build.bat'
let &errorformat='%f(%l)\ :\ %m'
map <F3> :wa <bar> !"../GameData/System/UCC.exe" make "ini=../../Code/UCC/UCC.ini" -noprompt <CR>
map <F4> :wa <bar> make debug <bar> :cwindow <CR>
map <F5> :!start "../GameData/System/CT.exe" testmap?game=mpgame.dmgame?maxplayers=32?Listen -windowed -log -nosaveconfig -rendev=opengl <CR>
