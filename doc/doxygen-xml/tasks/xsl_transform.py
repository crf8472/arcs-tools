# -*- coding: utf-8 -*-

## Applies an XSLT script on an input XML file and specifies the output.

import sys
import lxml.etree as ET

## Save a document by filename
def save_doc(doc, filename):
    with open(filename, 'w') as f:
        print(doc, file=f)
    return

## XSL-transform a single document by an already parsed XSLT transformation
def transform_doc(in_file, xslt_tf, out_file):
    doc = ET.parse(in_file)
    transformed_doc = xslt_tf(doc)
    save_doc(transformed_doc, out_file)
    return

## XSL-transform a single document
def xsl_transform(infile, xslFile, outfile):
    xslt = ET.parse(xslFile)
    transform = ET.XSLT(xslt)
    transform_doc(infile, transform, outfile)
    return

## XSL-transform a list of documents inplace
def xsl_transform_list_inplace(infiles, xslFile):
    xslt = ET.parse(xslFile)
    transform = ET.XSLT(xslt)
    for docfile in infiles:
        print("Transform: ", docfile)
        transform_doc(docfile, transform, docfile)
    return

## main

ARGC = len(sys.argv)

if   4 == ARGC:
    xsl_transform(sys.argv[1], sys.argv[2], sys.argv[3])
elif 3 == ARGC:
    infiles = sys.argv[1].split(';') # input is a CMake-style list
    xsl_transform_list_inplace(infiles, sys.argv[2])
else:
    print("Transform error: Require 2 or 3 arguments")

