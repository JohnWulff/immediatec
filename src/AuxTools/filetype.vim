" Vim filetype file
" Language:	iC
" Maintainer:	John E. Wulff <wulff.johne@gmail.com>
" Last Change:	2008 Jun 24 - 2015 Apr 6
" $Id: filetype.vim,v 1.4 2015/04/06 02:28:29 jw Exp $
" local filetype.vim must be in ~/.vim (ic.vim does not work there)

if exists("did_load_filetypes")
  finish
endif
augroup filetypedetect
  au! BufRead,BufNewFile *.ic		setfiletype ic
  au! BufRead,BufNewFile *.ica		setfiletype ic
  au! BufRead,BufNewFile *.ih		setfiletype ic
  au! BufRead,BufNewFile *.lst		setfiletype ic
  au! BufRead,BufNewFile *.lst6		setfiletype ic
augroup END
