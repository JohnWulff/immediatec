#!/bin/bash
# $Id: psed.sed,v 1.1 2005/02/01 20:25:01 jw Exp $
sed -ne '/icc\.v,v/ s/^.*\(icc.v,v [0-9][0-9]*\.[0-9][0-9]*\).*$/\1/p' scid.c
