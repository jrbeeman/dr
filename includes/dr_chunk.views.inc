<?php

function dr_chunk_views_data() {
  $data = array();

  // dr_chunk table
  $data['term_chunk']['table']['group'] = t('Taxonomy');
  $data['term_chunk']['table']['base'] = array(
    'field' => 'id',
    'title' => t('Term chunk'),
  );

  // term_chunk joins
  $data['term_chunk']['table']['join'] = array(
    'term_data' => array(
      'left_field' => 'tid',
      'field' => 'tid',
    ),
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
  );
  
  // term_chunk fields
  // tid field
  $data['term_chunk']['tid'] = array(
    'title' => t('Chunk Term ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'term_data',
      'base field' => 'tid',
      'label' => t('term'),
    ),
  );
  // nid field
  $data['term_chunk']['nid'] = array(
    'title' => t('Chunk Node ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'node',
      'base field' => 'nid',
      'label' => t('node'),
    ),

  );
  // html field
  $data['term_chunk']['html'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Chunk markup'),
    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => 'format',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );
  // text field
  $data['term_chunk']['text'] = array(
    'group' => t('Taxonomy'),
    'title' => t('Chunk text'),
    'field' => array(
      'handler' => 'views_handler_field_xss',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  return $data;
}