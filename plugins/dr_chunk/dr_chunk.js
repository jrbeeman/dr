var dr_chunk_editor = '';

Drupal.wysiwyg.plugins.dr_chunk = {

    attach: function(content, settings, instanceId) {
        // Prepare the text editor w/ the content menu
        Drupal.dr_chunk.prepare_node(instanceId);
        return content;
    }

}
