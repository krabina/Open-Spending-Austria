<?php

/**
 * Abstract blueprint for Dataset Donwloader
 *
 * Class OHDatasetDownloader
 */
abstract class OHDatasetDownloader {

    protected $name;

    protected $dataset;

    protected $charset;

    protected $format;

    protected $language;

    protected $raw;

    protected $transformed;

    /**
     * @param OHDataset $dataset
     * @param $charset
     * @param $format
     * @param $language
     */
    public function __construct(OHDataset $dataset, $charset, $format, $language) {
        $this->dataset = $dataset;
        $this->charset = $charset;
        $this->format = $format;
        $this->language = $language;
    }

    /**
     * @return mixed
     */
    public abstract function load();

    /**
     * @return mixed
     */
    public abstract function transform();

    /**
     * @throws Exception
     */
    public function serve() {
        if ($this->format == 'csv') {
            $this->serveCsv();
            return;
        }

        throw new Exception("{$this->name}: format {$this->format} not implemented!");
    }

    /**
     * @param int $filename
     * @param array $headings
     * @param string $delimiter
     */
    protected function serveCsv($filename = REQUEST_TIME, $headings = [], $delimiter = ';') {
        header("Content-Type:text/csv;charset={$this->charset}");
        header("Content-Disposition: attachment; filename=$filename.csv");

        // Write to output stream.
        $fh = fopen('php://output', 'w');
        // add header
        if (count($headings) > 0) {
            fputcsv($fh, $headings, $delimiter);
        }
        // add rows
        foreach ($this->transformed as $row) {
            fputcsv($fh, array_values($row), $delimiter);
        }
        // close
        fclose($fh);
    }
}