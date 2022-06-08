set wildignore+=*.obj,*.lib,*.exp,*.pdb,*.idb
set errorformat^=%f(%l):\ %m,%f(%l)\ :\ %m,%f\ :\ error\ %m,LINK\ :\ fatal\ error\ %m

map <silent> <F3> :wa <bar> let &makeprg='..\GameData\System\UCC.exe' <bar> make! make "ini=../../Code/UCC/UCC.ini" -noprompt -fullsourcepath -all <bar> :cwindow <CR><CR>
map <silent> <F4> :wa <bar> let &makeprg='build.bat' <bar> make! debug <bar> :cwindow <CR><CR>
map <F5> :!start "../GameData/System/CT.exe" dm_canyon?game=mpgame.dmgame?maxplayers=32?Listen -windowed -log -nosaveconfig -rendev=opengl <CR><CR>

augroup customhighlight
	autocmd!
	autocmd Syntax c,cpp syntax keyword cType BYTE SBYTE _WORD SWORD DWORD INT QWORD SQWORD ANSICHAR UNICHAR TCHAR UBOOL FLOAT DOUBLE SIZE_T BITFIELD
	autocmd Syntax c,cpp syntax keyword cDefine check checkSlow verify verifySlow debugf appErrorf debugfSlow appErrorfSlow guard guardSlow guardFunc guardFuncSlow unguard unguardSlow unguardf unguardfSlow appMalloc appFree appRealloc
	autocmd Syntax c,cpp syntax keyword cConstant INDEX_NONE
augroup end
