<?php
class Hwcoaclient extends CI_Controller {
	private $HnVpnKey = 'aipu_key=nobodyknows';
	static private $call_cnt = 0;
	const SOAP_LOCATION = 'http://127.0.0.1:8080/CodeIgniter_2.1.0/index.php/hwcoasvr';
	//const SOAP_LOCATION = 'http://11.255.41.153:90/program_new/hwcoasvr';
	//const SOAP_WSDL     = 'http://11.255.41.153:90/program_new/application/controllers/';
	const SOAP_URI      = 'http://127.0.0.1:8080/namespace';
	const CALL_CNT_FILE = 'logs/soap_call_cnt';
	const SOAP_DEVICE_TYPE = 'extelecom_type';
	const SOAP_DEVICE_ID   = 'extelecom_id';
	
	
	
	function __construct() {
		parent::__construct();
	}
	
    /* padding string to length */
    public function strpad($str, $pad_len, $pad_left = 1, $pad_char = ' ')
    {
    	$str1 = trim($str);
    	if ($pad_left == 1) {
    		$pad0 = '-';
    	}else{
    		$pad0 = '';
    	}
    	
  		if ($pad_char != ' ') {
  			$pad1 = $pad_char;
   		}else{
   			$pad1 = '';
   		}
 
    	$pad = sprintf('%'.$pad0.$pad1.$pad_len.'s', $str1);
    	return $pad;
    }
	
    public function genseq()
    {
    	$stamp = date('YmdHis');
    	
    	$file = dirname(__FILE__).'/'.'../'.self::CALL_CNT_FILE;
    	
    	if (!file_exists($file)) {
    		/* create soap call counter */
    		$fp = fopen($file, 'w');
    	    $cnt = 0;    	    
    	}else{
    		$fp = fopen($file, 'rw+');
    		$cnt = fread($fp, 10);
    		rewind($fp);
    	}
    	
    	$cnt++;
    	$seq = $cnt % 10000;

    	if ($seq == 0) {
    		$seq = 1;
    	}
    	
    	fputs($fp, $cnt, 10);
    	fclose($fp);    	
    	
    	$seq = sprintf('%04d', $seq);    	
    	return $stamp.$seq;
    }
    
	public function index() 
	{
		//var_dump(ob_get_contents());
        echo 'COA client should be called by URL/authuser, /queryuser, /moduser...';        
	}
	
	public function convert_bw($bw)
	{
		$upload = explode('U', $bw);
		$speed  = intval($upload[0]);
		if ($speed <= 1) { 
		    return -1;
		}
		$speed  = $speed/8;
		$val    = sprintf('%d'.'M', $speed);		
		return $val;
	}
	
	
	/* 
	 * call HW coa soap function 
	 * @params: $name - ADSL username
	 *          $pwd  - ADSL dialup password
	 * @return: 0 - auth user success
	 *          1 - auth fail
	 */
	public function authuser($name = 'test', $pwd = 'test-pwd') 
	{
		$client = new SoapClient(null, 
		                         array('location' => self::SOAP_LOCATION,
		                               'uri'      => self::SOAP_URI)
		                          );
		
		try {
			
			$ret = $client->authUser($this->strpad($this->genseq(), 18),
			                         $this->strpad(self::SOAP_DEVICE_TYPE, 100),
			                         $this->strpad(self::SOAP_DEVICE_ID, 100),
			                         $this->strpad($name, 61),
			                         $this->strpad($pwd, 16),
			                         '1');
			var_dump($ret);
			if ($ret['resultCode'] == 0) {
				echo "auth user=[$name] successful";
				return 0;
			}else{
				echo "auth user=$name failed!!!";
				return 1;
			}
		}catch(SoapFault $e) {
			var_dump($e->getMessage());
			return 1;
		}		
	}

	/* 
	 * call HW coa soap function 
	 * @params: $name - ADSL username
	 *          $pwd  - ADSL dialup password
	 * @return: 0 - auth user success
	 *          1 - auth fail
	 */
	
	public function queryuser($name = 'test')
	{

		try {
         	
	    	//$client = new SoapClient(null, 
		    //                         array('location' => self::SOAP_LOCATION,
		    //                        'uri'      => self::SOAP_URI));
		   	
	        //$wsdl = dirname(__FILE__).'/'.'HwCoaWSDLFile.wsdl';
    	    $wsdl = 'http://127.0.0.1:8080/wsdl/HwCoaWSDLFile.wsdl';
    	    $client = new SoapClient($wsdl, array("trace"=>true));
                        
			$ret = $client->userTemplateQuery($this->strpad($this->genseq(), 18),
			                                  $this->strpad(self::SOAP_DEVICE_TYPE, 100),
			                                  $this->strpad(self::SOAP_DEVICE_ID, 100),
			                                  $this->strpad($name.'@hw', 61));
		   $response_string=$client->__getLastResponse();			                                  
	        var_dump($response_string);	                                  
			if ($ret['resultCode'] == 0) {
				$bw = $ret['templateName'];
				$bw_name = $this->convert_bw($bw);
				echo "query user=[$name], bw=[$bw]($bw_name) successful.";
		        $result = array('name' => $name,
		                        'ret'  => 0,
		                        'msg'  => 'soap query ok',
		                        'bw_name' => $bw_name,
		                        'bw_val'  => $bw);		    		
			    return $result;				
			}else{
				echo "query user=[$name] failed!!!";
		        $result = array('name' => $name,
		                        'ret'  => 1,
		                        'msg'  => 'soap query got error',
		                        'bw_name' => '',
		                        'bw_val'  => '');		    		
			    return $result;				
			}

		}catch(SoapFault $e) {
			//echo $client->__getLastRequest();  
    		echo $client->__getLastResponse();  
			echo $e->getMessage();
			$result = array('name' => $name,
		                    'ret'  => 1,
			                'msg'  => 'soap query exception!',
		                    'bw_name' => '',
		                    'bw_val'  => '');		    		
			return $result;
		}			
	}
	
	public function moduser($name = 'test',
                            $bw   = '32U32D')
    {
    	if (empty($name) || empty($bw)) {
    		echo 'name or bw is NULL!';
    		return 1;
    	}
    	
    	$bw_val = $this->convert_bw($bw);
    	if ($bw_val == -1) {
    		echo "invalid bw_val=$bw";
    		return 1;    		
    	}
    	
		$client = new SoapClient(null, 
		                         array('location' => self::SOAP_LOCATION,
		                               'uri'      => self::SOAP_URI));
		
		try {
			$ip  = $this->input->ip_address();
			$ret = $client->modBandwidth($this->strpad($this->genseq(), 18),
			                             $this->strpad(self::SOAP_DEVICE_TYPE, 100),
			                             $this->strpad(self::SOAP_DEVICE_ID, 100),
			                             $this->strpad($ip, 15),
			                             $this->strpad($name.'@hw', 61),
			                             $this->strpad($bw, 40));
			var_dump($ret);
			if ($ret['resultCode'] == 0) {				
				echo "mod user=[$name], bw=[$bw] successful.";
				return 0;
			}else{
				$msg = $ret['resultMsg'];
				echo "mod user=[$name], bw=[$bw] failed, reason=[$msg]!!!";
				return 1;
			}	
		}catch(SoapFault $e) {
			var_dump($e->getMessage());
			return 1;
		}
    	
    }
    
    public function soaptest()
    {
    	try {
    		//$wsdl = dirname(__FILE__).'/'.'HwCoaWSDLFile.wsdl';
    	    // $client = new SoapClient($wsdl);
    	    $client = new SoapClient(null,
    	                             array('location' => self::SOAP_LOCATION,
		                                   'uri'      => self::SOAP_URI));
    	    
    	    /*                     
			$ret = $client->modBandwidth($this->strpad($this->genseq(), 18),
			                             $this->strpad(self::SOAP_DEVICE_TYPE, 100),
			                             $this->strpad(self::SOAP_DEVICE_ID, 100),
			                             $this->strpad('1.1.1.1', 15),
			                             $this->strpad('@hw', 61),
			                             $this->strpad('64U64D', 40));
			*/
			$ret = $client->authUser($this->strpad($this->genseq(), 18),
			                         $this->strpad(self::SOAP_DEVICE_TYPE, 100),
			                         $this->strpad(self::SOAP_DEVICE_ID, 100),
			                         $this->strpad('hw', 61),
			                         $this->strpad('pwd', 16),
			                         '1');
			                             	                         
			var_dump($ret);
    	    return 0;      
    	}catch(SoapFault $e) {
			var_dump($e->getMessage());
			return 1;
		}
    	
    	
    }
}