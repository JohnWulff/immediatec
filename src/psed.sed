#!/bin/bash
# $Id: psed.sed,v 1.3 2005/05/29 20:36:02 jw Exp $
base=${0%/*}
if [ -z $base ];then
    base='.'
fi
sed -ne '/icc\.v,v/ s/^.*icc.v,v \([0-9][0-9]*\.[0-9][0-9]*\).*$/icc.v \1/p' $base/scid.c
