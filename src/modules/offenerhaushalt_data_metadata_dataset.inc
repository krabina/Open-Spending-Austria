<?php

/**
 * Class OHDataset.
 *
 * An instance of this class represents one data set.
 *
 */
class OHDataset {

    protected $table;

    // relations
    protected $gemeinde;

    protected $id;
    protected $gid;
    protected $year;
    protected $source;
    protected $type;
    protected $code;
    protected $muid;
    protected $created_at;
    protected $updated_at;
    protected $metadata_identifier;

    /**
     * Constructor
     */
    public function __construct() {
        $this->table = OHTables::$METADATA_DATASETS;
    }

    /**
     * @param $id
     * @return null|OHDataset
     */
    public static function find($id) {
        $result = db_select(OHTables::$METADATA_DATASETS, 'd')
            ->fields('d')
            ->condition('id', $id)
            ->execute();

        $record = $result->fetchAssoc();

        if (!$record) {
            return null;
        }

        $dataset = new self();
        $dataset->fillFromDbRecord($record);

        return $dataset;
    }

    /**
     * @param $code
     * @return null|OHDataset
     */
    public static function findByCode($code) {
        $result = db_select(OHTables::$METADATA_DATASETS, 'd')
            ->fields('d')
            ->condition('code', $code)
            ->execute();

        $record = $result->fetchAssoc();

        if (!$record) {
            return null;
        }

        $dataset = new self();
        $dataset->fillFromDbRecord($record);

        return $dataset;
    }

    /**
     * @param $uuid
     * @return null|OHDataset
     */
    public static function findByUuid($uuid) {
        $result = db_select(OHTables::$METADATA_DATASETS, 'd')
            ->fields('d')
            ->condition('ogd_metadata_identifier', $uuid)
            ->execute();

        $record = $result->fetchAssoc();

        if (!$record) {
            return null;
        }

        $dataset = new self();
        $dataset->fillFromDbRecord($record);

        return $dataset;
    }

    /**
     * @param $gid
     * @param $year
     * @param $source
     * @param $type
     * @return null|string
     */
    public static function createCode($gid, $year, $source, $type) {
        if (!$gid || !$year || !$source || !$type) {
            return null;
        }

        return join('_',[ $gid, $year, $source, $type ]);
    }

    /**
     * @return string
     */
    public static function createUuid() {
        $data = openssl_random_pseudo_bytes(16);

        assert(strlen($data) == 16);

        $data[6] = chr(ord($data[6]) & 0x0f | 0x40); // set version to 0100
        $data[8] = chr(ord($data[8]) & 0x3f | 0x80); // set bits 6-7 to 10

        return vsprintf('%s%s-%s-%s-%s-%s%s%s', str_split(bin2hex($data), 4));
    }

    /**
     * @param $year
     * @param int $month
     * @param int $day
     * @param int $hour
     * @param int $minute
     * @param int $second
     * @return DateTime
     */
    public static function createDateTimeFromYear($year, $month = 1, $day = 1, $hour = 0, $minute = 0, $second = 0) {
        $dt = new DateTime();
        $dt->setDate($year, $month, $day);
        $dt->setTime($hour,$minute,$second);
        return $dt;
    }

    /**
     * @param $timestamp
     * @return DateTime
     */
    public static function createDateTimeFromTimestamp($timestamp) {
        return DateTime::createFromFormat('U', $timestamp);
    }

    /**
     * @param $gemeinde stdClass|integer
     * @param $year
     * @param $source
     * @param $type
     *
     * @return string
     */
    public static function createResourceUrl($gemeinde, $year, $source, $type) {
        global $base_url;

        if (is_int($gemeinde)) {
            $gemeinde = offenerhaushalt_get_gemeinde($gemeinde);
        }

        if (!$gemeinde || !$year || !$source || !$type) {
            return '';
        }

        /*
        $segments = [
            $base_url,
            'download',
            $gemeinde->machine_name,
            $year,
            $source,
            $type
        ];
        */
        $segments = [
            $base_url,
            'data',
            'download',
            self::createCode($gemeinde->gid,$year,$source,$type)
        ];

        return join('/',$segments);
    }

    /**
     * @param $gemeinde stdClass|integer
     *
     * @return string
     */
    public static function createMaintainerUrl($gemeinde) {
        global $base_url;

        if (is_int($gemeinde)) {
            $gemeinde = offenerhaushalt_get_gemeinde($gemeinde);
        }

        if (!$gemeinde) {
            return '';
        }

        $segments = [
            $base_url,
            'gemeinde',
            $gemeinde->machine_name
        ];

        return join('/',$segments);
    }

    /**
     * @param $record
     */
    protected function fillFromDbRecord($record) {
        $this->setId($record['id']);
        $this->setGid($record['gid']);
        $this->setYear($record['year']);
        $this->setSource($record['source']);
        $this->setType($record['type']);
        $this->setCode($record['code']);
        $this->setMuid($record['muid']);
        $this->setCreatedAt($record['created_at']);
        $this->setUpdatedAt($record['updated_at']);
        $this->setMetadataIdentifier($record['ogd_metadata_identifier']);
    }

    /**
     * Performs db_update on the data set.
     */
    protected function update() {
        $data = $this->attributesToArray();
        $data['updated_at'] = REQUEST_TIME;

        db_update($this->table)
            ->fields($data)
            ->condition('id',$this->id)
            ->execute();
    }

    /**
     * @return array
     */
    protected function attributesToArray() {

        return [
            'gid'    => $this->gid,
            'year'   => $this->year,
            'source' => $this->source,
            'type'   => $this->type,
            'code'   => $this->code,
            'muid'   => $this->muid,
            'ogd_metadata_identifier' => $this->metadata_identifier,

            'created_at' => $this->created_at,
            'updated_at' => $this->updated_at,
        ];
    }

    /**
     * Performs db_insert for this data set.
     *
     * @return DatabaseStatementInterface|int
     * @throws Exception
     */
    protected function create() {
        $this->code = self::createCode($this->gid, $this->year, $this->source, $this->type);
        $this->metadata_identifier = self::createUuid();

        $data = $this->attributesToArray();
        $data['created_at'] = REQUEST_TIME;
        $data['updated_at'] = REQUEST_TIME;

        $id = db_insert($this->table)
        ->fields($data)
        ->execute();

        return $id;
    }

    /**
     * Save a Dataset. If ID is not set, INSERT will be performed. UPDATE otherwise
     *
     * @return int|null
     */
    public function save() {
        if (!$this->id) {
            return $this->create();
        }

        $this->update();

        return null;
    }

    /**
     * @param OHMetadataConstants $constants
     * @param string $version
     * @return array
     * @throws Exception
     */
    public function toOgd(OHMetadataConstants $constants, $version = "2.4") {
        if ($version == "2.4") {
            return $this->toOgdV2_4($constants);
        }

        throw new Exception("OGD Version $version not supported.");
    }

    /**
     * Build meta data output, according to OGD-Metadaten_AT_2_4_2016_10_31.
     *
     * Mix information that is stored per record (dataset table) with static information (constants).
     *
     * Return exactly one resource per dataset.
     *
     * @param OHMetadataConstants $constants
     * @return array
     */
    protected function toOgdV2_4(OHMetadataConstants $constants) {
        $CONST = $constants->toKeyedArray($this->type);

        $constants_modified_at = OHMetadataConstants::lastUpdatedAt()->getTimestamp();
        $modified_at = $this->updated_at > $constants_modified_at ? $this->updated_at : $constants_modified_at;

        $ogd = [
            'metadata_identifier'   => $this->metadata_identifier,
            'metadata_modified'     => self::createDateTimeFromTimestamp($modified_at)->format('Y-m-d'),
            'metadata_original_portal'  => $CONST['metadata_original_portal'],
            'metadata_linkage_name' => $CONST['metadata_linkage_name'],
            'metadata_linkage'      => $CONST['metadata_linkage'],

            'title'                 => sprintf($CONST['title'],$this->gemeinde->name, $this->year),
            'en_title_and_desc'     => sprintf($CONST['en_title_and_desc'],$this->gemeinde->name),
            'description'           => $CONST['description'],
            'categorization'        => $CONST['categorization'],
            'keywords'              => array_map('trim', explode(',',$CONST['keywords'])),

            'begin_datetime'        => self::createDateTimeFromYear($this->year)->format('Y-m-d\TH:i:s'),
            'end_datetime'          => self::createDateTimeFromYear($this->year,12,31,23,59,59)->format('Y-m-d\TH:i:s'),

            'resources'             => [
                [
                    'resource_url'          => self::createResourceUrl($this->gemeinde, $this->year, $this->source, $this->type),
                    'resource_format'       => $CONST['resource_format'],
                    'resource_name'         => sprintf($CONST['title'],$this->gemeinde->name, $this->year),
                    'resource_created'      => self::createDateTimeFromTimestamp($this->created_at)->format('Y-m-d'),
                    'resource_lastmodified' => self::createDateTimeFromTimestamp($modified_at)->format('Y-m-d'),
                    'resource_language'     => $CONST['resource_language'],
                    'resource_encoding'     => $CONST['resource_encoding'],
                ]
            ],

            'maintainer'            => $this->gemeinde->name,
            'maintainer_link'       => self::createMaintainerUrl($this->gemeinde),

            'publisher'             => $CONST['publisher'],
            'publisher_link'        => $CONST['publisher_link'],
            'publisher_email'       => $CONST['publisher_email'],

            'license'               => $CONST['license'],
            'license_url'           => $CONST['license_url'],
            'license_citation'      => $CONST['license_citation'],

            'schema_name'           => $CONST['schema_name'],
            'schema_language'       => $CONST['schema_language'],
            'schema_characterset'   => $CONST['schema_characterset'],

            'geographic_toponym'    => $this->gemeinde->name,
            'update_frequency'      => $CONST['update_frequency'],
            'lineage_quality'       => $CONST[$this->source],
            'terms_url'             => $CONST['terms_url'],
            'attribute_description' => $CONST['attribute_description'],
        ];

        return $ogd;
    }

    /* -----------------------------------------------------------------------------------------------------------------
     * Getters / Setters
     * ---------------------------------------------------------------------------------------------------------------*/

    /**
     * @return mixed
     */
    public function getGid()
    {
        return $this->gid;
    }

    /**
     * @param integer $gid
     * @param boolean $autoload
     */
    public function setGid($gid, $autoload = true)
    {
        $this->gid = (int) $gid;

        // auto load relation ?
        if ($autoload) {
            $this->gemeinde = offenerhaushalt_get_gemeinde($gid);
        }
    }

    /**
     * @return mixed
     */
    public function getId()
    {
        return $this->id;
    }

    /**
     * @param mixed $id
     */
    public function setId($id)
    {
        $this->id = (int) $id;
    }

    /**
     * @return mixed
     */
    public function getYear()
    {
        return $this->year;
    }

    /**
     * @param mixed $year
     */
    public function setYear($year)
    {
        $this->year = (int) $year;
    }

    /**
     * @return mixed
     */
    public function getSource()
    {
        return $this->source;
    }

    /**
     * @param mixed $source
     */
    public function setSource($source)
    {
        $this->source = $source;
    }

    /**
     * @return mixed
     */
    public function getType()
    {
        return $this->type;
    }

    /**
     * @param mixed $type
     */
    public function setType($type)
    {
        $this->type = $type;
    }

    /**
     * @return mixed
     */
    public function getCode()
    {
        return $this->code;
    }

    /**
     * @param mixed $code
     */
    public function setCode($code)
    {
        $this->code = $code;
    }

    /**
     * @return mixed
     */
    public function getMuid()
    {
        return $this->muid;
    }

    /**
     * @param mixed $muid
     */
    public function setMuid($muid)
    {
        $this->muid = $muid ? (int) $muid : null;
    }

    /**
     * @return mixed
     */
    public function getCreatedAt()
    {
        return $this->created_at;
    }

    /**
     * @param mixed $created_at
     */
    public function setCreatedAt($created_at)
    {
        $this->created_at = (int) $created_at;
    }

    /**
     * @return mixed
     */
    public function getUpdatedAt()
    {
        return $this->updated_at;
    }

    /**
     * @param mixed $updated_at
     */
    public function setUpdatedAt($updated_at)
    {
        $this->updated_at = (int) $updated_at;
    }

    /**
     * @return mixed
     */
    public function getMetadataIdentifier()
    {
        return $this->metadata_identifier;
    }

    /**
     * @param mixed $metadata_identifier
     */
    public function setMetadataIdentifier($metadata_identifier)
    {
        $this->metadata_identifier = $metadata_identifier;
    }

}