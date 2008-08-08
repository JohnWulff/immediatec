" Vim filetype file
" Language:	iC
" Maintainer:	John E. Wulff <john@je-wulff.de>
" Last Change:	2008 Jun 24
" $Id: filetype.vim,v 1.3 2008/06/24 14:37:44 jw Exp $

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
