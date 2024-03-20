# -*- coding: utf-8 -*-

## Applies dedup_index.xsl on an input XML file.

## This is intended to be called by a CMake target to deduplicate the compounds
## in doxygen's index.xml in a dedicated build step.

import sys
import lxml.etree as ET

def xsl_transform(inDoc, xslFile, outDoc):
    doc = ET.parse(inDoc)
    xslt = ET.parse(xslFile)
    transform = ET.XSLT(xslt)
    transformed_doc = transform(doc)
    with open(outDoc, 'w') as f:
        print(transformed_doc, file=f)
    return

def doxy_deduplicate_index(filename):
    xsl_transform(filename,
            "@PROJECT_DOC_DIR@/thirdparty/doxygen/dedup_index.xsl", filename)
    print('Removed duplicated <compound> elements in index.xml')

doxy_deduplicate_index(sys.argv[1])

