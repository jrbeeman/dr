<?php

function dr_user_taxonomy_activity_by_date($uid, $vid) {
  $user = user_load(array('uid' => $uid));
  $vocabulary = taxonomy_vocabulary_load($vid);
  $terms = taxonomy_get_tree($vid);
  $series = array();
  $categories = array();
  $date_start = strtotime(format_date($user->created, 'custom', 'Y-m-d'));
  $date_end = NULL; 
  $sql = "SELECT td.tid,
            td.name, 
            COUNT(c.cid) AS num,
            FROM_UNIXTIME(c.timestamp, '%%Y-%%m-%%d') AS day
          FROM {term_data} td
            LEFT JOIN {term_comment} tc ON tc.tid = td.tid
            LEFT JOIN {comments} c ON c.cid = tc.cid AND c.uid = %d
          WHERE td.vid = %d
          GROUP BY tid, day
          ORDER BY day, tid";
  $result = db_query($sql, $uid, $vid);
  while ($row = db_fetch_object($result)) {
    $date = strtotime($row->day);
    $series[check_plain($row->name)][$date] += $row->num;
    if ($date > $date_end) {
      $date_end = $date;
    }
  }

  if (!$date_end) {
    return;
  }
  // Fill in dates
  foreach ($series as $term => $dates) {
    //dr_fill_array_dates($dates, $date_start, $date_end);
    //dr_filter_array($dates, 50);
    $new_dates = array();
    foreach ($dates as $date => $count) {
      if ($date && $count) {
        $new_dates[format_date($date, 'custom', 'Y-m-d')] = $count;
      }
    }
    $series[$term] = $new_dates;
  }
  $chart_data = array();
  $category_totals = array();
  foreach ($series as $category => $data) {
    foreach ($data as $key => $value) {
      $category_totals[$category] += $value;
      $chart_data[] = array($category, $key, $value);
      //$chart_data[] = array($category, $key, $category_totals[$category]);
    }
  }
  $info->data = $chart_data;
  $info->chart_type = 'Multi-series Line 2D';
  $info->settings = array(
    'caption' => t('Activity by term in !vocab', array('!vocab' => $vocabulary->name)),
    'animation' => 0,
    'shownames' => 0,
    'showValues' => 0,
    'showAnchors' => 0,
    'numVDivLines' => 4,
    'numdivlines' => 4,
    'showAreaBorder' => 0,
    'areaAlpha' => 50,
    'showShadow' => 0,
    'xAxisName' => format_date($date_start, 'custom', 'Y-m-d') .' to '. format_date($date_end, 'custom', 'Y-m-d'),
    'yAxisName' => t('Posts'),
    'decimalPrecision' => 0,
  );
  $info->attributes = array(
  );
  $info->width = 400;
  $info->height = 300 + (count($terms) * 5);

  $out .= theme('fusionchart', $info);
  return $out;


}

function dr_thread_taxonomy_activity_by_date($nid, $vid) {
  $node = node_load($nid);
  $vocabulary = taxonomy_vocabulary_load($vid);
  $terms = taxonomy_get_tree($vid);
  $series = array();
  $categories = array();
  $date_start = strtotime(format_date($node->created, 'custom', 'Y-m-d'));
  $date_end = NULL; 
  $sql = "SELECT td.tid,
            td.name, 
            COUNT(c.cid) AS num,
            FROM_UNIXTIME(c.timestamp, '%%Y-%%m-%%d') AS day
          FROM {term_data} td
            INNER JOIN {term_comment} tc ON tc.tid = td.tid
            INNER JOIN {comments} c ON c.cid = tc.cid AND c.nid = %d
          WHERE td.vid = %d
          GROUP BY tid, day
          ORDER BY day, tid";
  $result = db_query($sql, $nid, $vid);
  while ($row = db_fetch_object($result)) {
    $date = strtotime($row->day);
    $series[$row->name][$date] += $row->num;
    if ($date > $date_end) {
      $date_end = $date;
    }
  }

  if (!$date_end) {
    return;
  }

  // Fill in dates
  foreach ($series as $term => $dates) {
    dr_fill_array_dates($dates, $date_start, $date_end);
    dr_filter_array($dates, 50);
    $new_dates = array();
    foreach ($dates as $date => $count) {
      $new_dates[format_date($date, 'custom', 'Y-m-d')] = $count;
    }
    $series[$term] = $new_dates;
  }
  
  $chart_data = array();
  $category_totals = array();
  foreach ($series as $category => $data) {
    foreach ($data as $key => $value) {
      $category_totals[$category] += $value;
      $chart_data[] = array($category, $key, $value);
      //$chart_data[] = array($category, $key, $category_totals[$category]);
    } 
  }

  $info->data = $chart_data;
  $info->chart_type = 'Multi-series Line 2D';
  $info->settings = array(
    'caption' => t('Activity by term in !vocab', array('!vocab' => $vocabulary->name)),
    'animation' => 0,
    'shownames' => 0,
    'showValues' => 0,
    'showAnchors' => 0,
    'numVDivLines' => 4,
    'numdivlines' => 4,
    'showAreaBorder' => 0,
    'areaAlpha' => 50,
    'showShadow' => 0,
    'xAxisName' => format_date($date_start, 'custom', 'Y-m-d') .' to '. format_date($date_end, 'custom', 'Y-m-d'),
    'yAxisName' => t('Posts'),
    'decimalPrecision' => 0,
  );
  $info->attributes = array(
  );
  $info->width = 400;
  $info->height = 300 + (count($terms) * 5);

  $out .= theme('fusionchart', $info);
  return $out;

 
}


