<?php

class OHStatistikAustriaRechnungsabschlussDownloader extends OHDatasetDownloader {

    /**
     * @param OHDataset $dataset
     * @param $charset
     * @param $format
     * @param $language
     */
    public function __construct(OHDataset $dataset, $charset, $format, $language) {
        parent::__construct($dataset, $charset, $format, $language);

        $this->name = 'Statistik Austria Rechnungsabschluss Downloader';
    }

    public static $attributes = [
        'de' => [
            'gid'       => 'gkz',
            'year'      => 'jahr',
            'hh'        => 'haushaltskonto-hinweis',
            'hh_name'   => 'haushaltskonto-hinweis-name',
            'ans3l3'    => 'haushaltskonto-ansatz',
            'ans_name'  => 'ansatzbezeichnung',
            'post3l3'   => 'haushaltskonto-post',
            'post_name' => 'kontenbezeichnung',
            'soll'      => 'soll-rj',
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

        $select = db_select(OHTables::$FINANZDATEN_STA,'d');

        $select->join(OHTables::$HAUSHALTSHINWEIS,'hh','d.hh = hh.hhid');
        $select->join(OHTables::$ANS3,'ans','d.ans3l3 = ans.ans3l3id');
        $select->join(OHTables::$POST3,'post', 'd.post3l3 = post.post3l3id');

        $select->addField('d','gid');
        $select->addField('d','year');
        $select->addField('d','hh');
        $select->addField('hh','name','hh_name');
        $select->addField('d','ans3l3');
        $select->addField('ans','name','ans_name');
        $select->addField('d','post3l3');
        $select->addField('post','name','post_name');
        $select->addField('d','soll');

        $select = $select->condition('d.gid',$this->dataset->getGid())
            ->condition('d.year',$this->dataset->getYear());

        $result = $select->execute();

        // reset and fill
        $this->raw = [];

        while ($row = $result->fetchAssoc()) {
            $this->raw[] = $row;
        }

        return $this;
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
            $row['soll'] = str_replace('.', ',', $row['soll']);
        }

        return $row;
    }

    /**
     * @param int $filename
     * @param array $headings
     * @param string $delimiter
     */
    protected function serveCsv($filename = REQUEST_TIME, $headings = [], $delimiter = ';') {
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
            'STA',
            'RA'
        ];

        return join('_',$segments);
    }
}