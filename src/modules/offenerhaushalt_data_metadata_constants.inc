<?php

/**
 * Class OHMetadataConstants
 *
 * Provide functionality to read meta data constants.
 */
class OHMetadataConstants {

    protected $constants;

    public static $var_name_updated_at = 'offenerhaushalt_data_metadata_updated_at';

    public function __construct() {
        $this->all();
    }

    /**
     * @return DateTime
     */
    public static function lastUpdatedAt() {
        if (!variable_get(self::$var_name_updated_at,null)) {
            variable_set(self::$var_name_updated_at,REQUEST_TIME);
        }

        return OHDataset::createDateTimeFromTimestamp(variable_get(self::$var_name_updated_at));
    }

    /**
     *
     */
    protected function all() {
        $query = db_select(OHTables::$METADATA_CONSTANTS,'c')->fields('c');
        $result = $query->execute();

        while ($row = $result->fetchAssoc()) {
            $this->constants[] = $row;
        }
    }

    /**
     * @param $type
     * @return array
     */
    public function toKeyedArray($type) {
        $arr = [];

        foreach($this->constants as $constant) {
            if ($constant['type'] != null && $constant['type'] != $type) {
                continue;
            }

            $arr[$constant['key']] = trim($constant['value']);
        }

        return $arr;
    }

}