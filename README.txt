* HEADS UP!
This module is under heavy development and really not suited for public use, particularly outside the context of a Decoder Ring Drupal site.

* REQUIREMENTS
  - jQuery Update module
  - QueryPath library downloaded and extracted to /sites/all/libraries/QueryPath

* INSTALLATION
  1. Create database tables (INSTALL.txt - will be moved into the modules themselves soon).
  2. Add the following line to the singular theme's page.tpl.php file (in D7 this won't be necessary because we can control CSS file weight)
     <link type="text/css" rel="stylesheet" media="screen" href="/sites/all/modules/dr/custom.css" />
