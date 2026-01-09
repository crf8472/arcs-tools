#!/usr/bin/env bash
##
## THIS IS FOR DEMO PURPOSES ONLY.
##
## Copy the m.css cascading stylesheet files used by m.css in this directory.
## The set of this files is a custom style for m.css.

CSS_FILE=(
	'm-dark'
	'm-theme-dark'
	'm-grid'
	'm-components'
	'm-layout'
	'pygments-console'
	'pygments-dark'
	'm-documentation'
)

for FILE in ${CSS_FILE[@]}; do
	cp ../../../../build/_deps/m_css-src/css/${FILE}.css .
done

