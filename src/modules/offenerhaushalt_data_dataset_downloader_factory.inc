<?php

/**
 * Class OHDatasetDownloaderFactory
 */
class OHDatasetDownloaderFactory {

    /**
     * @param $dataset_code
     * @param string $charset
     * @param string $format
     * @param string $language
     *
     * @return null|OHDatasetDownloader
     *
     * @throws Exception
     */
    public static function create($dataset_code, $charset = 'utf-8', $format = 'csv', $language = 'de') {

        $dataset = OHDataset::findByCode($dataset_code);

        // not found? return early
        if (!$dataset) {
            return null;
        }

        if ($dataset->getSource() == OFFENERHAUSHALT_DATA_SRC_STA &&
            $dataset->getType() == OFFENERHAUSHALT_DATA_TYPE_RA) {
            return new OHStatistikAustriaRechnungsabschlussDownloader($dataset, $charset, $format, $language);
        }

        if ($dataset->getSource() == OFFENERHAUSHALT_DATA_SRC_GEM &&
            $dataset->getType() == OFFENERHAUSHALT_DATA_TYPE_RA) {
            return new OHGemeindeRechnungsabschlussDownloader($dataset, $charset, $format, $language);
        }

        if ($dataset->getSource() == OFFENERHAUSHALT_DATA_SRC_GEM &&
            $dataset->getType() == OFFENERHAUSHALT_DATA_TYPE_VA) {
            return new OHGemeindeVoranschlagDownloader($dataset, $charset, $format, $language);
        }

        throw new Exception("Unable to create Dataset Downloader for {$dataset->getSource()}_{$dataset->getType()}!");
    }
}