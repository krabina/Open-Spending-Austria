<?php

class OHGemeindeRechnungsabschlussDownloader extends OHDatasetDownloader {

    /**
     * @param OHDataset $dataset
     * @param $charset
     * @param $format
     * @param $language
     */
    public function __construct(OHDataset $dataset, $charset, $format, $language) {
        parent::__construct($dataset, $charset, $format, $language);

        $this->name = 'Gemeinde (GHD) Rechnungsabschluss Downloader';
    }

    public static $attributes = [
        'de' => [
            'gkz',
            'jahr',
            'haushaltskonto-hinweis',
            'haushaltskonto-hinweis-name',
            'haushaltskonto-ansatz',
            'ansatzbezeichnung',
            'haushaltskonto-post',
            'kontenbezeichnung',
            'haushaltskonto-sonstige-untergliederung',
            'kennzeichen-fuer-verguetungen',
            'anfaengliche-reste-rj',
            'soll-rj',
            'ist-rj',
            'voranschlag-rj',
            'rechnungsergebnis-soll-vj',
            'mittelfristige-finanzplanung-soll-folgejahr+1',
            'mittelfristige-finanzplanung-soll-folgejahr+2',
            'mittelfristige-finanzplanung-soll-folgejahr+3',
            'mittelfristige-finanzplanung-soll-folgejahr+4',
        ]
    ];

    /**
     * @return $this
     * @throws Exception
     */
    public function load() {
        if ($this->language != 'de') {
            throw new Exception("{$this->name}: language {$this->language} not implemented!");
        }

        $select = db_select(OHTables::$FINANZDATEN_GHD_02,'d');

        $select->join(OHTables::$HAUSHALTSHINWEIS,'hh','d.hh = hh.hhid');

        $select->addField('d','gid');
        $select->addField('d','year');
        $select->addField('d','hh');
        $select->addField('hh','name','hh_name');
        $select->addField('d','ans');
        $select->addField('d','ans_text');
        $select->addField('d','post');
        $select->addField('d','post_text');
        $select->addField('d','sonst');
        $select->addField('d','compensation');
        $select->addField('d','init_out_payments');
        $select->addField('d','soll');
        $select->addField('d','ist');
        $select->addField('d','voranschlag_total');
        $select->addField('d','soll_previous');
        $select->addField('d','mfp_soll_plus_one');
        $select->addField('d','mfp_soll_plus_two');
        $select->addField('d','mfp_soll_plus_three');
        $select->addField('d','mfp_soll_plus_four');

        $select = $select->condition('d.muid',$this->dataset->getMuid())
            ->condition('d.gid',$this->dataset->getGid())
            ->condition('d.year',$this->getDataYear());

        $result = $select->execute();

        // reset and fill
        $this->raw = [];

        while ($row = $result->fetchAssoc()) {
            $this->raw[] = $row;
        }

        return $this;
    }

    /**
     * @return mixed
     */
    protected function getDataYear() {
        return $this->dataset->getYear();
    }

    /**
     * @return $this
     * @throws Exception
     */
    public function transform() {
        if ($this->charset != 'utf-8') {
            throw new Exception("{$this->name}: charset {$this->charset} not implemented!");
        }

        foreach($this->raw as $row) {
            // we actually create a (transformed) copy of each row
            $this->transformed[] = $this->transformRow($row);
        }

        return $this;
    }

    /**
     * @param $row
     * @return mixed
     */
    protected function transformRow($row) {
        if ($this->language == 'de') {
            $row['init_out_payments']   = str_replace('.', ',', $row['init_out_payments']);
            $row['soll']                = str_replace('.', ',', $row['soll']);
            $row['ist']                 = str_replace('.', ',', $row['ist']);
            $row['voranschlag_total']   = str_replace('.', ',', $row['voranschlag_total']);
            $row['soll_previous']       = str_replace('.', ',', $row['soll_previous']);
            $row['mfp_soll_plus_one']   = str_replace('.', ',', $row['mfp_soll_plus_one']);
            $row['mfp_soll_plus_two']   = str_replace('.', ',', $row['mfp_soll_plus_two']);
            $row['mfp_soll_plus_three'] = str_replace('.', ',', $row['mfp_soll_plus_three']);
            $row['mfp_soll_plus_four']  = str_replace('.', ',', $row['mfp_soll_plus_four']);
        }

        return $row;
    }

    /**
     *
     */
    protected function serveCsv() {
        $filename = $this->makeFilename();

        $headings = array_values(self::$attributes[$this->language]);

        parent::serveCsv($filename, $headings);
    }

    /**
     * @return string
     */
    protected function makeFilename() {
        $segments = [
            'offenerhaushalt',
            $this->dataset->getGid(),
            $this->dataset->getYear(),
            'GEM',
            'RA'
        ];

        return join('_',$segments);
    }
}