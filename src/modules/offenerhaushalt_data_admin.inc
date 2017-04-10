<?php

/**
 * @param $form
 * @param $form_state
 * @return mixed
 */
function offenerhaushalt_data_admin_constants($form, &$form_state) {

    $constantsUpdatedAt = OHMetadataConstants::lastUpdatedAt();

    $html = '';
    $html.= '<p>Letztes Metadaten Fix-Texte Update: '.$constantsUpdatedAt->format('d.m.Y h:i').'</p>';
    $html.= '<p style="font-size: 110%"><strong>ACHTUNG:</strong> Durch das Neu-Setzen des Zeitstempels wird das <strong>modified_date</strong> aller zum aktuellen Zeitpunkt existierender Metadata-Datasets aktualisiert.</p>';

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Fix-Texte Zeitstempel neu setzen'),
        '#prefix' => $html,
    );

    return $form;
}

/**
 * @param $form
 * @param $form_state
 * @return bool
 */
function offenerhaushalt_data_admin_constants_validate($form, &$form_state) {
    return true;
}

/**
 * @param $form
 * @param $form_state
 */
function offenerhaushalt_data_admin_constants_submit($form, &$form_state) {
    variable_set(OHMetadataConstants::$var_name_updated_at,REQUEST_TIME);

    drupal_set_message('Zeitstempel aktualisiert.');
}

/**
 * @param $form
 * @param $form_state
 * @return mixed
 */
function offenerhaushalt_data_admin_update_metadata_form($form, &$form_state) {
    $warning_text = '<p>Diese Funktion kann sowohl für das erstmalige Generieren von Metadaten (bereits existierende Statistik Austria bzw. GHD Daten), als auch für das jährliche Einspielen für Statistik Austria Metadaten verwendet werden.</p>';

    $form['year'] = array(
        '#title' => "Jahr auswählen",
        '#description' => t("Metadaten für das gewählte Jahr erstellen."),
        '#type' => 'textfield',
        '#prefix' => '<h3>Metadaten generieren</h3>'.$warning_text,
        '#attributes' => array(
            'style'=>'width: 50px',
        ),
    );
    $form['include_ghd'] = array(
        '#title' => 'GHD Uploads berücksichtigen',
        '#description' => t("Ist die Checkbox aktiv, werden GHD Uploads zum gewählten Jahr ebenfalls als Datenset angelegt. (Achtung: dies ist nur beim Initialien Anlegen der Metadaten notwendig, danach 'verwalten' sich die Metadaten der GHD Uploads selbstständig)"),
        '#type' => 'checkbox',
        '#default_value' => false,
        '#suffix' => '<br>'
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Metadaten generieren'),
    );

    return $form;
}

/**
 * @param $form
 * @param $form_state
 * @return bool
 */
function offenerhaushalt_data_admin_update_metadata_form_validate($form, &$form_state) {

    if (!$form_state['values']['year']) {
        drupal_set_message('Year is required','error');
        return false;
    }

    return true;
}

/**
 * @param $form
 * @param $form_state
 */
function offenerhaushalt_data_admin_update_metadata_form_submit($form, &$form_state) {

    $year = $form_state['values']['year'];

    $result = db_select(OHTables::$FINANZDATEN_STA, 'f');
    $result->join(OHTables::$GEMEINDEN,'g','f.gid = g.gid');
    $result = $result->fields('f', [ 'gid', 'year' ])
        ->condition('g.published_at',0,'>')
        ->condition('f.year',$year)
        ->groupBy('f.gid')
        ->groupBy('f.year')
        ->execute();

    $count = 0;

    while ($row = $result->fetchAssoc()) {
        $count++;

        $dataset = new OHDataset();
        $dataset->setGid($row['gid']);
        $dataset->setYear($row['year']);
        $dataset->setType(OFFENERHAUSHALT_DATA_TYPE_RA);
        $dataset->setSource(OFFENERHAUSHALT_DATA_SRC_STA);
        $dataset->save();
    }

    drupal_set_message("Created $count Statistik Austria Metadata-datasets. (Year: $year)");

    if ($form_state['values']['include_ghd']) {
        $count = 0;
        $row = null;

        $result = db_select(OHTables::$MANAGED_UPLOADS, 'mu');
        $result->join(OHTables::$MANAGED_UPLOAD_YEAR,'muy','mu.muid = muy.muid');
        $result->join(OHTables::$MANAGED_UPLOAD_BUDGETTYPE,'mub','mu.muid = mub.muid');
        $result->join(OHTables::$GEMEINDEN,'g','mu.gid = g.gid');
        $result = $result->fields('mu', [ 'muid', 'gid' ])
            ->fields('muy',[ 'year' ])
            ->fields('mub',[ 'budgettype' ])
            ->condition('g.published_at',0,'>')
            ->condition('mu.status','active')
            ->condition('muy.year',$year)
            ->execute();

        if ($result) {
            while ($row = $result->fetchAssoc()) {
                $count++;

                $dataset = new OHDataset();
                $dataset->setGid($row['gid']);
                $dataset->setYear($row['year']);
                $dataset->setType(strtoupper($row['budgettype']));
                $dataset->setSource(OFFENERHAUSHALT_DATA_SRC_GEM);
                $dataset->setMuid($row['muid']);
                $dataset->save();
            }
        }

        drupal_set_message("Created $count Gemeinde (GHD, Satzart 02) Metadata-datasets. (Year: $year)");
    }
}

/**
 * @return mixed
 */
function offenerhaushalt_data_admin_settings_form() {
    $form['email-receiver'] = array(
        '#title' => "Email-Empfänger für User Registrierungs Anfragen",
        '#type' => 'textfield',
        '#size' => 64,
        '#default_value' => variable_get('offenerhaushalt_data_user_request_receiver'),
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Speichern'),
    );

    return $form;
}

/**
 * @param $form
 * @param $form_state
 * @return bool
 */
function offenerhaushalt_data_admin_settings_form_validate($form, &$form_state) {
    if (!valid_email_address($form_state['values']['email-receiver'])) {
        form_set_error('', t('Email ist ungültig.'));
        return false;
    }

    return true;
}

/**
 * @param $form
 * @param $form_state
 */
function offenerhaushalt_data_admin_settings_form_submit($form, &$form_state) {
    variable_set('offenerhaushalt_data_user_request_receiver',$form_state['values']['email-receiver']);

    drupal_set_message('Einstellungen wurden gespeichert.');
}