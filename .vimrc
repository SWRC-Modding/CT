let g:ctrlp_custom_ignore='.obj\|.exp\|.pdb\|.idb\|.lib'
set errorformat=%f(%l):\ %m,%f(%l)\ :\ %m,%f\ :\ error\ %m

map <F3> :wa <bar> let &makeprg='..\GameData\System\UCC.exe' <bar> make! make "ini=../../Code/UCC/UCC.ini" -noprompt -fullsourcepath -all <bar> :cwindow <CR><CR>
map <F4> :wa <bar> let &makeprg='build.bat' <bar> make! debug <bar> :cwindow <CR><CR>
map <F5> :!start "../GameData/System/CT.exe" dm_canyon?game=mpgame.dmgame?maxplayers=32?Listen -windowed -log -nosaveconfig -rendev=opengl <CR><CR>
