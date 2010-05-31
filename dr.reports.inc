<?php

/** ---- Activity reports ---- **/

function dr_block_collection_activity() {
  return _dr_block_object_activity(DR_COLLECTION_TYPE);
}

function dr_block_post_activity() {
  return _dr_block_object_activity(DR_POST_TYPE);
}

function dr_block_user_activity() {
  return _dr_block_object_activity(DR_USER_TYPE);
}

function _dr_block_object_activity($type) {
  $allowed_types = array(DR_COLLECTION_TYPE, DR_POST_TYPE, DR_USER_TYPE);

  // Only show when viewing a node
  if (arg(0) == 'node' && is_numeric(arg(1)) && in_array($type, $allowed_types)) {
    $node = node_load(arg(1));

    $dates = array();
    $date_start = NULL;
    $date_end = NULL;

    // Collection nodes
    if ($type == DR_COLLECTION_TYPE) {
      $sql = "SELECT
                n.nid, n.vid,
                cck.field_dr_date_value AS date
              FROM {node} n
                INNER JOIN {content_type_dr_post} cck ON n.vid = cck.vid
                INNER JOIN {content_field_dr_collection} f ON n.vid = f.vid
              WHERE 
                f.field_dr_collection_nid = %d";
      $result = db_query($sql, $node->nid);
    }

    // Post nodes
    if ($type == DR_POST_TYPE) {
      $sql = "SELECT
                n.nid, n.vid,
                cck.field_dr_date_value AS date
              FROM {node} n
                INNER JOIN {content_type_dr_post} cck ON n.vid = cck.vid
              WHERE cck.field_dr_parent_nid = %d";
      $result = db_query($sql, $node->nid);
    }

    // User nodes
    if ($type == DR_USER_TYPE) {
      $sql = "SELECT
                n.nid, n.vid,
                cck.field_dr_date_value AS date
              FROM {node} n
                INNER JOIN {content_type_dr_post} cck ON n.vid = cck.vid
              WHERE cck.field_dr_user_nid = %d";
      $result = db_query($sql, $node->nid);
    }

    while ($row = db_fetch_object($result)) {
      $timestamp = strtotime(format_date($row->date, 'custom', 'Y-m-d'));
      // Negotiate the start and end date
      if ($timestamp < $date_start || !$date_start) {
        $date_start = $timestamp;
      }
      if ($timestamp > $date_end || !$date_end) {
        $date_end = $timestamp;
      }
      $dates[$timestamp]++;
    }

    return array(
      'subject' => t('Activity'),
      'content' => _dr_chart_activity_by_date($dates, $date_start, $date_end),
    );
  }
}

/** ---- Taxonomy reports ---- **/

function dr_block_collection_taxonomy() {
  return _dr_block_object_taxonomy(DR_COLLECTION_TYPE);
}

function dr_block_post_taxonomy() {
  return _dr_block_object_taxonomy(DR_POST_TYPE);
}

function dr_block_user_taxonomy() {
  return _dr_block_object_taxonomy(DR_USER_TYPE);
}

function _dr_block_object_taxonomy($type) {
  $allowed_types = array(DR_COLLECTION_TYPE, DR_POST_TYPE, DR_USER_TYPE);

  // Only show when viewing a node
  if (arg(0) == 'node' && is_numeric(arg(1)) && in_array($type, $allowed_types)) {
    $node = node_load(arg(1));

    // Check if this is a child post of a larger thread and, therefore, has no thread activity
    if (($node->type == DR_POST_TYPE) && ($node->field_dr_parent[0]['nid'])) {
      return;
    }

    $out = '';

    // Get all the vocabularies for the post node type
    $vocabs = taxonomy_get_vocabularies(DR_POST_TYPE);
    foreach ($vocabs as $vid => $vocab) {
      $info = new stdClass();
      $data = array();
      $count_total = 0;

      // Collection nodes
      if ($type == DR_COLLECTION_TYPE) {
        $sql = "SELECT
                  td.tid AS tid, 
                  td.name AS term_name,
                  COUNT(tc.tid) AS num
                FROM term_chunk tc
                  INNER JOIN {node} n ON n.nid = tc.nid
                  INNER JOIN {content_field_dr_collection} f ON f.vid = n.vid
                  INNER JOIN {term_data} td ON tc.tid = td.tid
                WHERE f.field_dr_collection_nid = %d
                  AND td.vid = %d
                GROUP BY tc.tid
                ORDER BY td.weight ASC";
        $result = db_query($sql, $node->nid, $vid);
      }

      // Post nodes
      if ($type == DR_POST_TYPE) {
        $sql = "SELECT
                  td.tid AS tid, 
                  td.name AS term_name,
                  COUNT(tc.tid) AS num
                FROM term_chunk tc
                  INNER JOIN {content_type_dr_post} cck ON tc.nid = cck.nid
                  INNER JOIN {node} n ON n.vid = cck.vid
                  INNER JOIN {term_data} td ON tc.tid = td.tid
                WHERE (cck.field_dr_parent_nid = %d OR cck.nid = %d)
                  AND td.vid = %d
                GROUP BY tc.tid
                ORDER BY td.weight ASC";
        $result = db_query($sql, $node->nid, $node->nid, $vid);
      }

      // User nodes
      if ($type == DR_USER_TYPE) {
        $sql = "SELECT
                  td.tid AS tid, 
                  td.name AS term_name,
                  COUNT(tc.tid) AS num
                FROM term_chunk tc
                  INNER JOIN {content_type_dr_post} cck ON tc.nid = cck.nid
                  INNER JOIN {node} n ON n.vid = cck.vid
                  INNER JOIN {term_data} td ON tc.tid = td.tid
                WHERE cck.field_dr_user_nid = %d
                  AND td.vid = %d
                GROUP BY tc.tid
                ORDER BY td.weight ASC";
        $result = db_query($sql, $node->nid, $vid);
      }

      // Parse the results
      while ($row = db_fetch_object($result)) {
        $data[] = array($row->term_name, $row->num);
        $count_total += $row->num;
      }

      // Don't render a chart if the data is empty
      if (!count($data)) {
        continue;
      }
      $height = count($chart_data) * 25;
      if ($height < DR_CHART_HEIGHT) {
        $height = DR_CHART_HEIGHT;
      }

      $info->data = $data;
      $info->chart_type = 'Bar 2D';
      $info->settings = array(
        'caption' => t('Terms in !vocab', array('!vocab' => $vocab->name)),
        'bgColor' => DR_CHART_BG,
        'shownames' => 1,
        'showValues' => 0,
        'showAnchors' => 0,
        'animation' => 0,
        'yAxisMinValue' => 0,
        'yAxisMaxValue' => $count_total,
        'numdivlines' => 1,
      );
      $info->attributes = array();
      $info->width = DR_CHART_WIDTH;
      $info->height = $height;
      
      $out .= theme('fusionchart', $info);
    }
  }
  return array(
    'subject' => t('Taxonomy term usage'),
    'content' => $out,
  );
}

function _dr_chart_activity_by_date($dates, $date_start, $date_end) {
  // Fill the array out with any empty dates
  dr_fill_array_dates($dates, $date_start, $date_end);
  $dates_clone = array_keys($dates);
  $date_start = array_shift($dates_clone);
  $date_end = array_pop($dates_clone);

  // Filter the results to a reasonable number ~100
  dr_filter_array($dates, 100);

  $chart_data = array();
  $chart_data_total = array();
  $total = 0;
  foreach ($dates as $date => $count) {
    $total += $count;
    $chart_data[] = array(
      format_date($date, 'custom', 'Y-m-d'),
      $count,
    );
    $chart_data_total[] = array(
      format_date($date, 'custom', 'Y-m-d'),
      $total,
    );
  }

  $info->data = $chart_data;
  $info->chart_type = 'Line 2D';
  $info->settings = array(
    'caption' => t('Activity over time'),
    'bgColor' => DR_CHART_BG,
    'animation' => 0,
    'shownames' => 0,
    'showValues' => 0,
    'showAnchors' => 1,
    'numVDivLines' => 1,
    'numdivlines' => 1,
    'xAxisName' => format_date($date_start, 'custom', 'Y-m-d') .' to '. format_date($date_end, 'custom', 'Y-m-d'),
    'yAxisName' => t('Count'),
    'decimalPrecision' => 0,
  );
  $info->attributes = array(
    'color' => array('ff3333'),
  );
  $info->width = DR_CHART_WIDTH;
  $info->height = DR_CHART_HEIGHT;

  $out .= theme('fusionchart', $info);
  return $out;
}

/** ---- Taxonomy activity reports ---- **/

function dr_block_collection_taxonomy_activity() {
  return _dr_block_object_taxonomy_activity(DR_COLLECTION_TYPE);
}

function dr_block_post_taxonomy_activity() {
  return _dr_block_object_taxonomy_activity(DR_POST_TYPE);
}

function dr_block_user_taxonomy_activity() {
  return _dr_block_object_taxonomy_activity(DR_USER_TYPE);
}

function _dr_block_object_taxonomy_activity($type) {
  $allowed_types = array(DR_COLLECTION_TYPE, DR_POST_TYPE, DR_USER_TYPE);

  // Only show when viewing a node
  if (arg(0) == 'node' && is_numeric(arg(1)) && in_array($type, $allowed_types)) {
    $node = node_load(arg(1));

    // Check if this is a child post of a larger thread and, therefore, has no thread activity
    if (($node->type == DR_POST_TYPE) && ($node->field_dr_parent[0]['nid'])) {
      return;
    }

    $out = '';

    // Get all the vocabularies for the post node type
    $vocabs = taxonomy_get_vocabularies(DR_POST_TYPE);
    foreach ($vocabs as $vid => $vocab) {
      
      $terms = array();
      $series = array();
      $date_start = NULL;
      $date_end = NULL;

      // Collection nodes
      if ($type == DR_COLLECTION_TYPE) {
        $sql = "SELECT
                  td.tid AS tid, td.name AS term_name,
                  cck.field_dr_date_value AS date
                FROM {term_data} td
                  INNER JOIN {term_chunk} tc ON td.tid = tc.tid
                  INNER JOIN {node} n ON tc.nid = n.nid
                  INNER JOIN {content_field_dr_collection} f ON n.vid = f.vid
                  INNER JOIN {content_type_dr_post} cck ON n.vid = cck.vid
                WHERE 
                  f.field_dr_collection_nid = %d
                  AND td.vid = %d";
        $result = db_query($sql, $node->nid, $vid);
      }      
      
      // Post nodes
      if ($type == DR_POST_TYPE) {
        $sql = "SELECT
                  td.tid AS tid, td.name AS term_name,
                  cck.field_dr_date_value AS date
                FROM {term_data} td
                  INNER JOIN {term_chunk} tc ON td.tid = tc.tid
                  INNER JOIN {node} n ON tc.nid = n.nid
                  INNER JOIN {content_type_dr_post} cck ON n.vid = cck.vid
                WHERE 
                  (cck.field_dr_parent_nid = %d OR cck.nid = %d)
                  AND td.vid = %d";
        $result = db_query($sql, $node->nid, $node->nid, $vid);
      }      

      // User nodes
      if ($type == DR_USER_TYPE) {
        $sql = "SELECT
                  td.tid AS tid, td.name AS term_name,
                  cck.field_dr_date_value AS date
                FROM {term_data} td
                  INNER JOIN {term_chunk} tc ON td.tid = tc.tid
                  INNER JOIN {node} n ON tc.nid = n.nid
                  INNER JOIN {content_type_dr_post} cck ON n.vid = cck.vid
                WHERE 
                  cck.field_dr_user_nid = %d
                  AND td.vid = %d";
        $result = db_query($sql, $node->nid, $vid);
      }

      while ($row = db_fetch_object($result)) {
        $timestamp = strtotime(format_date($row->date, 'custom', 'Y-m-d'));
        // Negotiate the start and end date
        if ($timestamp < $date_start || !$date_start) {
          $date_start = $timestamp;
        }
        if ($timestamp > $date_end || !$date_end) {
          $date_end = $timestamp;
        }
        $dates[$timestamp]++; // Keep a dates array so that later we can keep items in series in sync
        $series[$row->tid][$timestamp]++;
        $terms[$row->tid] = $row->term_name;
      }

      if (!count($series)) {
        continue;
      }

      $chart_data = array();

      // Fill out the dates array
      dr_fill_array_dates($dates, $date_start, $date_end);
      $dates_clone = array_keys($dates);
      $date_start = array_shift($dates_clone);
      $date_end = array_pop($dates_clone);

      // Build the chart data
      foreach ($series as $tid => $series_dates) {
        dr_fill_array_dates($series_dates, $date_start, $date_end);
        dr_filter_array($series_dates, 100);
        foreach ($series_dates as $key => $value) {
          $date_nice = format_date($key, 'custom', 'Y-m-d');
          $chart_data[] = array($terms[$tid], $date_nice, $value);
        }
      }
      
      // Output the chart
      $info->data = $chart_data;
      $info->chart_type = 'Multi-series Line 2D';
      $info->settings = array(
        'caption' => t('Activity by term in !vocab', array('!vocab' => $vocab->name)),
        'bgColor' => DR_CHART_BG,
        'animation' => 0,
        'shownames' => 0,
        'showValues' => 0,
        'showAnchors' => 1,
        'numVDivLines' => 4,
        'numdivlines' => 4,
        'showAreaBorder' => 0,
        'areaAlpha' => 50,
        'showShadow' => 0,
        'xAxisName' => format_date($date_start, 'custom', 'Y-m-d') .' to '. format_date($date_end, 'custom', 'Y-m-d'),
        'yAxisName' => t('Count'),
        'decimalPrecision' => 0,
      );
      $info->attributes = array();
      $info->width = DR_CHART_WIDTH;
      $info->height = DR_CHART_HEIGHT + (count($terms) * 5);
      $out .= theme('fusionchart', $info);
    }

    return array(
      'subject' => t('Taxonomy activity'),
      'content' => $out,
    );
  }
}