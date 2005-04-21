#!/bin/bash
# $Id: psed.sed,v 1.2 2005/02/03 09:45:28 jw Exp $
sed -ne '/icc\.v,v/ s/^.*icc.v,v \([0-9][0-9]*\.[0-9][0-9]*\).*$/icc.v \1/p' scid.c
