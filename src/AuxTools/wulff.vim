" Vim color file
" Maintainer:	Bram Moolenaar <Bram@vim.org> - John E. Wulff <wulff.johne@gmail.com>
" Last Change:	2001 Jul 23 - 2015 Apr 6
" $Id: wulff.vim,v 1.2 2015/04/06 02:22:14 jw Exp $

" Copy to /usr/share/vim/vim73/colors/wulff.vim

" This is the default color scheme.  It doesn't define the Normal
" highlighting, it uses whatever the colors used to be.

" Set 'background' back to the default.  The value can't always be estimated
" and is then guessed.
hi clear Normal
set bg&

" Remove all existing highlighting and set the defaults.
hi clear

" Load the syntax highlighting defaults, if it's enabled.
if exists("syntax_on")
  syntax reset
endif

let colors_name = "wulff"

" Groups used in the 'highlight' and 'guicursor' options default value.
hi Search term=reverse ctermbg=Cyan ctermfg=Black guibg=Yellow guifg=Black

" vim: sw=2
