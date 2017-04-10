<?php

/**
 * Batch Operation Callback
 *
 * Each batch operation callback will iterate over and over until $context['finished'] is set to 1.
 *
 * @param $source
 * @param $params
 * @param $all_access boolean
 * @param $destination
 * @param $settings
 * @param $context
 */
function oh_data_batch_process($source, $params, $all_access, $destination, $settings, &$context) {
    module_load_include('inc', 'offenerhaushalt', 'offenerhaushalt_finance');

    if ($all_access) {
        $params['all_access'] = true;
    }

    // results are shared between operations
    if (!isset($context['results']['init'])) {
        oh_data_batch_init_context_results($context);
    }

    // sandbox is shared between requests, but not between operations
    if (!isset($context['sandbox']['progress'])) {
        oh_data_batch_init_context_sandbox(offenerhaushalt_finance_query($source, $params, 0, 1, true)->fetchField(), $context);

        $context['results']['files'][] = $destination;
    }

    // one process for each block
    $result = offenerhaushalt_finance_query($source, $params, $context['sandbox']['offset'], $settings['blockSize']);
    $data   = oh_data_transform_result($params, $result);

    if ($data == null) { // no data returned ? exit now
        $context['finished'] = 1;
        return;
    }

    // start writing
    if (!file_exists($settings['dir'])) {
        mkdir($settings['dir']);
    }

    $headings = oh_data_get_headings();

    oh_data_write_csv($destination, $data, $headings, $context['sandbox']['offset'] == 0 ? 'w' : 'a');

    // update the batch context, with progress info
    oh_data_batch_update_context($source, count($data), $context);

    // Inform the batch engine that we are not finished,
    // and provide an estimation of the completion level we reached.
    if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
        $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
    }
}

/**
 * @param $max
 * @param $context
 */
function oh_data_batch_init_context_sandbox($max, &$context) {
    $context['sandbox']['progress'] = 0; // how many records a processed
    $context['sandbox']['max'] = $max;
    $context['sandbox']['offset'] = 0;
}

/**
 * @param $context
 */
function oh_data_batch_init_context_results(&$context) {
    $context['results']['init'] = 1;
    $context['results'][OFFENERHAUSHALT_DATA_SRC_STA] = 0;
    $context['results'][OFFENERHAUSHALT_DATA_SRC_GEM] = 0;
    $context['results']['files'] = [];
}

/**
 * @param $source
 * @param $count
 * @param $context
 */
function oh_data_batch_update_context($source, $count, &$context) {
    $context['sandbox']['progress'] += $count;
    $context['sandbox']['offset'] += $count;

    $context['results'][$source] += $count;

    if ($source == OFFENERHAUSHALT_DATA_SRC_STA) {
        $context['message'] = t('Records (Statistics Austria): %i', array('%i' => $context['sandbox']['progress']));
    }
    if ($source == OFFENERHAUSHALT_DATA_SRC_GEM) {
        $context['message'] = t('Records (Municipal household data file): %i', array('%i' => $context['sandbox']['progress']));
    }
}

/**
 * @param $success
 * @param $results
 * @param $operations
 */
function oh_data_batch_finished($success, $results, $operations) {
    if ($success) {
        // TODO this would be correct place to write some data to database
        //$message = t('@count items successfully processed:', array('@count' => $results['count']));
        //drupal_set_message($message);
    }
    else {
        // An error occurred.
        // $operations contains the operations that remained unprocessed.
        $error_operation = reset($operations);
        $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
        drupal_set_message($message, 'error');
    }

    //drupal_set_message('Batch finished');
}

/**
 * @param $settings
 * @param $context
 */
function oh_data_batch_compress($settings, &$context) {
    $context['message'] = t('Compressing data').'...';

    if ($context['results'][OFFENERHAUSHALT_DATA_SRC_STA] == 0 && $context['results'][OFFENERHAUSHALT_DATA_SRC_GEM] == 0) {
        $context['message'] = t('No data available');
    }

    // do compression here
    $context['results']['compressed'] = oh_data_compress_files($context['results']['files'], $settings['dir'], true);

    $context['finished'] = 1;
}