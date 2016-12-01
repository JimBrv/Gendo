<?php
class hwcoasvr extends CI_Controller {
	private $api_keys = array('aipu_key=nobodyknows');
	const SOAP_URI  = 'http://127.0.0.1:8080/namespace';
	const HW_COA_2M = '16U16D';
	const HW_COA_4M = '32U32D';
	const HW_COA_6M = '48U48D';
	const HW_COA_8M = '64U64D';
	const HW_COA_10M= '80U80D';

	
    function  __construct() {
        parent::__construct();
        $this->load->database();
    }
 
    public function index() {
        //$server = new SoapServer(null, 
        //                         array('uri' => self::SOAP_URI));
        
    	//$wsdl = dirname(__FILE__).'/'.'HwCoaWSDLFile.wsdl';
		//var_dump(ob_get_contents());
    	$wsdl = 'http://127.0.0.1:8080/wsdl/HwCoaWSDLFile.wsdl';
    	$server = new SoapServer($wsdl);
        $server->setObject($this);
        $server->handle();
    }
    
    /* padding string to length */
    private function strpad($str, $pad_len, $pad_left = 1, $pad_char = ' ')
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
 
    public function authUser($sequenceNo, 
                      $srcDeviceType,
                      $srcDeviceId,
                      $loginName,
                      $password,
                      $needUserInfo) 
    {
    	log_message('debug', "auth_user: seq=$sequenceNo, name=$loginName, pwd=$password");
    	    
        $result = array('sequenceNo'   => $this->strpad($sequenceNo, 18),
                        'resultCode'   => 0,
                        'loginName'    => $this->strpad($loginName, 61),
                        'serviceLevel' => '1',
                        'status'       => '1',
                        'substate'     => '1',
                        'accountLeft'  => 0);
        return $result;
    } 
    
    public function userTemplateQuery($sequenceNo, 
                               $srcDeviceType, 
                               $srcDeviceId, 
                               $loginName)
    {
        log_message('debug', "query_user: seq=$sequenceNo, name=$loginName");
        $result = array('sequenceNo'   => $this->strpad($sequenceNo, 18),
                        'resultCode'   => 0,
                        'templateName' => $this->strpad(self::HW_COA_4M, 40));  
    	return $result;
    }
    
    public function modBandwidth($sequenceNo, 
                          $srcDeviceType,
                          $srcDeviceId,
                          $userIP,
                          $loginName,
                          $templateName)
    {
        log_message('debug', "mod_user: seq=$sequenceNo, ip=$userIP, name=$loginName, bw=$templateName");
    	    
        $result = array('sequenceNo'   => $this->strpad($sequenceNo, 18),
                        'resultCode'   => 0,
                        'resultMsg'    => $this->strpad('modify bw ok', 50));
    	return $result;    	                 	
    }
}