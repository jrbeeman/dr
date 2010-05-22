Drupal.dr = {};

Drupal.dr.initialize = function() {
  $('a.dr-node, a.dr-comment').each(function() {
    var $dialog = $('<div></div>');
    var $link = $(this).one('click', function() {
      $dialog
        .load($link.attr('href'))
        .dialog({
          title: $link.attr('title'),
          position: ['right', 'top'],
          width: $(window).width() / 2,
          height: $(window).height(),
          modal: false,
          draggable:false,
          dialogClass: 'dr-dialog',
          open: function(event, ui) {
            $link.parents('.comment').animate({ backgroundColor: '#ffffcc' }, 500); 
          },
          close: function(event, ui) {
            $link.parents('.comment').animate({ backgroundColor: '#f3f3f3' }, 500);
          }
        });

      $link.click(function() {
        $dialog.dialog('open');
        return false;
      });
      return false;
    });
  });
  $('.flag-reviewed-comment .unflag-action').parents('.comment').fadeTo('slow', 0.5);
  // Hide stats
  $('#thread-statistics .pane-content').hide();
  $('#thread-statistics h2').wrapInner('<a></a>').bind('click', function() {
    $('#thread-statistics .pane-content').toggle();
    return false;
  });
}

                        

if (Drupal.jsEnabled) {
  $(document).ready(function() {
    Drupal.dr.initialize();
  });
}
