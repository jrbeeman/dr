<?php
function dr_chunk_dr_chunk_plugin($editor = NULL) {
  $plugins['dr_chunk'] = array(
    'module' => 'dr_chunk',
    'title' => t('DR chunk tagger'),
    'description' => t('Tag chunks'),
    'vendor url' => 'http://jeffbeeman.com',
//    'icon file' => 'dr_chunk.gif',
//    'icon title' => t('Tag chunks of text with taxonomy terms'),
    'settings' => array('terms' => dr_chunk_all_terms()),
  );
  drupal_add_css('sites/all/themes/fusion/fusion_core/css/style.css', 'theme');
  return $plugins;
}