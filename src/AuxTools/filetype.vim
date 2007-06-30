" Vim filetype file
" Language:	iC
" Maintainer:	John E. Wulff <john@je-wulff.de>
" Last Change:	2007 Apr 11
" $Id: filetype.vim,v 1.2 2007/04/12 17:08:28 jw Exp $

if exists("did_load_filetypes")
  finish
endif
augroup filetypedetect
  au! BufRead,BufNewFile *.ic		setfiletype ic
  au! BufRead,BufNewFile *.ih		setfiletype ic
  au! BufRead,BufNewFile *.lst		setfiletype ic
  au! BufRead,BufNewFile *.lst6		setfiletype ic
augroup END
