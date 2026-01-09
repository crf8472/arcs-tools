# Custom colors (under construction)

Implement overriding the default m.css dark theme by a custom theme.

A full customization of an m.css theme would entail modifying the following
files:

  - m-dark.css
  - m-theme-dark.css
  - m-grid.css
  - m-components.css
  - m-layout.css
  - pygments-console.css
  - pygments-dark.css
  - m-documentation.css

This replaces m-dark.css and m-theme-dark.css by m-custom.css and
m-theme-custom.css.

m-custom.css and m-theme-custom.css are compiled as a single css file, specified
in the Doxyfile and deployed in the documentation output directory.

m-custom.css has only one modified line: background color is red. This is just
to test that the correct file has been loaded.

