<?php

class OHGemeindeVoranschlagDownloader extends OHGemeindeRechnungsabschlussDownloader {

    /**
     * @param OHDataset $dataset
     * @param $charset
     * @param $format
     * @param $language
     */
    public function __construct(OHDataset $dataset, $charset, $format, $language) {
        parent::__construct($dataset, $charset, $format, $language);

        $this->name = 'Gemeinde (GHD) Voranschlag Downloader';
    }

    /**
     * @return int
     */
    protected function getDataYear() {
        // voranschlag data records are stored with
        // one year prior to <year of voranschlag>
        return ((int)$this->dataset->getYear()) - 1;
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
            'VA'
        ];

        return join('_',$segments);
    }
}