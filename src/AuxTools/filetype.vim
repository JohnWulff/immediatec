" Vim filetype file
" Language:	iC
" Maintainer:	John E. Wulff <john@je-wulff.de>
" Last Change:	2007 Apr 11
" $Id: filetype.vim,v 1.1 2007/04/11 15:53:22 jw Exp $

if exists("did_load_filetypes")
  finish
endif
augroup filetypedetect
  au! BufRead,BufNewFile *.ic		setfiletype ic
  au! BufRead,BufNewFile *.ih		setfiletype ic
  au! BufRead,BufNewFile *.lst		setfiletype ic
augroup END
