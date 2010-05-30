<?php

function dr_report_taxonomy_filter($form_state, $vid = NULL) {
  $vocabs = taxonomy_get_vocabularies();
  $options = array();
  foreach ($vocabs as $vocab) {
    $options[$vocab->vid] = $vocab->name;
  }
  $form = array(
    '#prefix' => '<div class="container-inline">', 
    '#suffix' => '</div>',
  );
  $form['vid'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary'),
    '#options' => $options,
    '#default_value' => $vid,
  );
  $form['limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Limit'),
    '#default_value' => $_GET['limit'],
    '#size' => 5,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

function dr_report_taxonomy_filter_submit(&$form, $form_values) {
  drupal_goto('report/taxonomy/'.$form_values['values']['vid'], 'limit='.$form_values['values']['limit']);
}

function dr_report_user_taxonomy($uid, $vid, $ignore_zero = FALSE) {
  $vocabulary = taxonomy_vocabulary_load($vid);
  // Get nodes
  $sql = "SELECT td.tid AS tid,
          td.name AS name,
          COUNT(n.nid) AS num
          FROM {term_data} td
            LEFT JOIN {term_node} tn ON tn.tid = td.tid
            LEFT JOIN {node} n ON n.vid = tn.vid AND n.uid = %d 
          WHERE td.vid = %d
          GROUP BY td.name ";
  $result = db_query($sql, $uid, $vid);
  $data = array();
  $count_total = 0;
  while ($row = db_fetch_object($result)) {
    $data[$row->tid] = array(
      'name' => $row->name,
      'value' => $row->num,
    );
    $count_total += $row->num;
  }
  // Get comments
  $sql = "SELECT td.tid AS tid,
          td.name AS name,
          COUNT(c.cid) AS num
          FROM {term_data} td
            LEFT JOIN {term_comment} tc ON tc.tid = td.tid
            LEFT JOIN {comments} c ON c.cid = tc.cid AND c.uid = %d
          WHERE td.vid = %d
          GROUP BY td.name";
  $result = db_query($sql, $uid, $vid);
  while ($row = db_fetch_object($result)) {
    if (isset($data[$row->tid])) {
      $data[$row->tid]['value'] += $row->num;
    }
    else {
      $data[$row->tid] = array(
        'name' => $row->name,
        'value' => $row->num ? $row->num : 0,
      );
    }
    $count_total += $row->num;
  }

  $chart_data = array();
  foreach ($data as $key => $value) {
    if ($ignore_zero && !$value['value']) {
      continue;
    }
    else {
      $chart_data[] = array($value['name'], $value['value']);
    }
  }

  if (!count($chart_data) || !$count_total) {
    return;
  }
  
  $height = count($chart_data) * 25;
  if ($height < 200) {
    $height = 200;
  }

  $info->data = $chart_data;
  $info->chart_type = 'Bar 2D';
  $info->settings = array(
    'caption' => t('Terms in !vocab', array('!vocab' => $vocabulary->name)),
    'showValues' => 0,
    'animation' => 0,
    'numdivlines' => 1,
  );
  $info->attributes = array();
  $info->width = 400;
  $info->height = $height;
  return theme('fusionchart', $info);
}

function dr_report_taxonomy($vid = NULL) {
  $vocabulary = taxonomy_vocabulary_load($vid);
  $limit = $_GET['limit'];
  $header = array(
    array('data' => t('Term'), 'field' => 'term_data_name'),
    array('data' => t('Threads'), 'field' => 'count_nid'),
    array('data' => t('Comments')),
  );

  $sql = "SELECT td.tid AS tid, 
          td.name AS term_data_name,
          COUNT(n.nid) AS count_nid
          FROM {node} n 
            LEFT JOIN {term_node} tn ON n.vid = tn.vid
            LEFT JOIN {term_data} td ON tn.tid = td.tid
          WHERE (n.status <> 0) 
            AND (td.vid = %d)
          GROUP BY td.name ";

  $tablesort = tablesort_sql($header);
  $sql .= $tablesort;

  if ($limit) {
    $sql .= " LIMIT %d";
  }

  $result = db_query($sql, $vid, $limit);

  $data = array();
  $count_total = 0;
  while ($row = db_fetch_object($result)) {
    $data[] = array(
      'tid' => $row->tid,
      'name' => $row->term_data_name,
      'count_nid' => $row->count_nid,
    );
    $count_total += $row->count_nid;
  }

  $rows = array();
  $chart_data = array();
  foreach ($data as $row) {
    $sql = "SELECT COUNT(cid) 
            FROM {comments} c
            INNER JOIN {term_node} tn ON c.nid = tn.nid AND tn.tid = %d";
    $count_cid = (int)db_result(db_query($sql, $row['tid']));
    $rows[] = array($row['name'], $row['count_nid'], $count_cid);
    $chart_data[] = array($row['name'], $row['count_nid'] + $count_cid);
  }

  $info->data = $chart_data;
  $info->chart_type = 'Bar 2D';
  $info->settings = array(
    'caption' => t('Posts by term in !vocab', array('!vocab' => $vocabulary->name)),
    'showValues' => 0,
    'animation' => 0,
    'numdivlines' => 1,
    'decimalPrecision' => 0,
  );
  $info->attributes = array();
  $info->width = 600;
  $info->height = 400;

  drupal_set_title(t('Report for terms in vocabulary %game', array('%game' => $vocabulary->name)));
  $out .= '<div style="text-align:center;">'. theme('fusionchart', $info) .'</div>';
  $out .= drupal_get_form('dr_report_taxonomy_filter', $vid);
  $out .= theme('table', $header, $rows);
  return $out;
}

function dr_thread_taxonomy_totals($nid, $vid) {
  $node = node_load($nid);
  $vocabulary = taxonomy_vocabulary_load($vid);

  $sql = "SELECT td.name AS name,
          COUNT(c.cid) AS num
          FROM {comments} c
            LEFT JOIN {term_comment} tc ON c.cid = tc.cid
            LEFT JOIN {term_data} td ON tc.tid = td.tid
          WHERE c.nid = %d
            AND td.vid = %d
          GROUP BY td.name
          ORDER BY num DESC
          LIMIT 10";
  $result = db_query($sql, $nid, $vid);

  $data = array();
  $count_total = 0;
  while ($row = db_fetch_object($result)) {
    $data[] = array($row->name, $row->num);
    $count_total += $row->num;
  }

  if (!count($data)) {
    return;
  }
  $info->data = $data;
  $info->chart_type = 'Bar 2D';
  $info->settings = array(
    'caption' => t('Terms in !vocab', array('!vocab' => $vocabulary->name)),
    'shownames' => 1,
    'showValues' => 0,
    'animation' => 0,
    'yAxisMinValue' => 0,
    'yAxisMaxValue' => $node->comment_count,
    'numdivlines' => 1,
  );
  $info->attributes = array();
  $info->width = '400';
  $info->height = 300;

  $out .= theme('fusionchart', $info);
  return $out;
}

function dr_user_activity_by_date($uid) {
  $user = user_load(array('uid' => $uid));
  $dates = array();
  $date_start = strtotime(format_date($user->created, 'custom', 'Y-m-d'));
  $total = 0;
  
  // Nodes
  $sql = "SELECT
          COUNT(nid) AS num,
          FROM_UNIXTIME(created, '%%Y-%%m-%%d') AS day
          FROM {node} n
          WHERE uid = %d
          GROUP BY day
          ORDER BY day";
  $result = db_query($sql, $uid);
  while ($row = db_fetch_object($result)) {
    $timestamp = strtotime($row->day);
    $dates[$timestamp] += $row->num;
    $date_end = $timestamp;
  }
  
  // Comments
  $sql = "SELECT
          COUNT(cid) AS num,
          FROM_UNIXTIME(timestamp, '%%Y-%%m-%%d') AS day
          FROM {comments} c
          WHERE uid = %d
          GROUP BY day
          ORDER BY day";
  $result = db_query($sql, $uid);
  while ($row = db_fetch_object($result)) {
    $timestamp = strtotime($row->day);
    $dates[$timestamp] += $row->num;
    if ($timestamp > $date_end) {
      $date_end = $timestamp;
    }
  }

  return dr_chart_activity_by_date($dates, $date_start, $date_end);
}

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

function dr_thread_activity_by_date($nid) {
  $node = node_load($nid);
  $date_start = strtotime(format_date($node->created, 'custom', 'Y-m-d'));
  $dates[$date_start] = 1;
  $date_end = NULL;

  $sql = "SELECT 
          COUNT(cid) AS num, 
          FROM_UNIXTIME(timestamp, '%%Y-%%m-%%d') AS day 
          FROM {comments} 
          WHERE nid = %d 
          GROUP BY day 
          ORDER BY day";
  $result = db_query($sql, $nid);
  while ($row = db_fetch_object($result)) {
    $timestamp = strtotime($row->day);
    $dates[$timestamp] += $row->num;
    $date_end = $timestamp;
  }

  return dr_chart_activity_by_date($dates, $date_start, $date_end);
}

function dr_chart_activity_by_date($dates, $date_start, $date_end) {
  // Fill the array out with any empty dates
  dr_fill_array_dates($dates, $date_start, $date_end);

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
    'animation' => 0,
    'shownames' => 0,
    'showValues' => 0,
    'showAnchors' => 0,
    'numVDivLines' => 1,
    'numdivlines' => 1,
    'lineColor' => 'ff3333',
    'xAxisName' => format_date($date_start, 'custom', 'Y-m-d') .' to '. format_date($date_end, 'custom', 'Y-m-d'),
    'yAxisName' => t('Posts'),
    'decimalPrecision' => 0,
  );
  $info->attributes = array(
    'color' => array('ff3333'),
  );
  $info->width = 400;
  $info->height = 200;

  $out .= theme('fusionchart', $info);
  return $out;
}
