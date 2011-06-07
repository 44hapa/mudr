<?php



define('MUD_ADDRESS',               "balderdash.ru");
define('MUD_PORT',                  9000);
define('MEMMORY_KEY_FROMBALDER',    'fromBalder');


class telnetConnect {

    /**
     *
     * @var telnetConnect
     */
    protected static $instance;

    private $socket;
    
    /**
     *
     * @var Memcache
     */
    private $memory;


    private $in_charset     = 'windows-1251';
//    private $in_charset     = 'KOI8';
    private $out_charset    = 'UTF-8';
    private $socLenght      = 4000;


    private function  __construct() {
    }

    /**
     *
     * @return telnetConnect
     */
    public function botConnect(){
        $str    = "";

        while (strpos($str, "Select your codepage") === false) {
            $str    = socket_read ($this->socket, $this->socLenght);
            echo $str;
         }

        socket_write($this->socket, "1\n");

        while (strpos($str, "Под каким именем ты здесь будешь?") === false) {
            $str    = socket_read ($this->socket, $this->socLenght);
            $str    = iconv($this->in_charset, $this->out_charset, $str);
            echo $str;
         }

         return $this;
    }


    /**
     *
     * @param string $str
     * @return telnetConnect
     */
    public function socketWrite($str){
        $str    = iconv($this->out_charset, $this->in_charset, $str);
        socket_write($this->socket, $str."\r\n");
        return $this;
    }


        private function deleteTelnetChar($str){

        $telnetChar     = array(
            chr(0),
            chr(1),
        );

        $empty  = array(
            'chr(0)',
            'chr(1)',
        );
        
        return  str_replace($telnetChar, $empty, $str);

    }

    public function socketRead(){

        while (true){
            $mem    = $this->readMemoryOnce();
            if($mem != ""){
                socket_set_block($this->socket);
                $this->socketWrite($mem);
                socket_set_nonblock($this->socket);
            }

            $this->writeToFile("./cp1251.txt");
            sleep(1);
            
            $str    = socket_read($this->socket, $this->socLenght);

            $str    = $this->deleteTelnetChar($str);
            $str    = iconv($this->in_charset, $this->out_charset, $str);

            echo $str;
        }
    }


    private function writeToFile($fileName){
            $fh = fopen($fileName, "a+");
            fwrite($fh, $str);
            fclose($fh);
    }

    private function readMemoryOnce(){
        $mem        = $this->memory->get(MEMMORY_KEY_FROMBALDER);
        $this->memory->flush();
        return $mem;
    }

    /**
     *
     * @return telnetConnect
     */
    static public function instance(){

        if ( is_null(self::$instance) ) {
            self::$instance = new self;
            $socket         = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
            socket_set_nonblock($socket);
            socket_connect($socket, MUD_ADDRESS, MUD_PORT);
            self::$instance->socket     = $socket;
            self::$instance->memory     = new Memcache();
            self::$instance->memory->addServer('localhost', 11211);

            self::$instance->readMemoryOnce();
            

        }
        else{
            die ('FUCK!! IN [FILE: '.__FILE__.'] AT [LINE: '.__LINE__.']');
        }

        return self::$instance;
    }

}

?>