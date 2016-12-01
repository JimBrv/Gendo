<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Vpnadduser extends CI_Controller {
	private $api_keys = array('aipu_key=nobodyknows');
	
	
    function  __construct() {
        parent::__construct();
        $this->load->database();
    }
 
    public function index() {
        $server = new SoapServer(null, 
                                 array('uri' => "http://127.0.0.1:8080/namespace"));
        $server->setObject($this);
        $server->handle();
    }
 
    function sayHello($name) {
        $salute = "Hi " . $name . ", it's working!";
        return $salute;
    } 
    
    function HnVpnAddUser($key, $name, $pwd, $month_traffic = 10, $expire = '2015-01-01 00:00:00')
    {
    	$valid = FALSE;
    	log_message("debug", "adduser: key=$key, name=$name, pwd=$pwd");
    	
    	if (empty($key) || empty($name) || empty($pwd)) {
    		$ret = array('ret' => '1', 'msg' => 'input string get null!');
    		return $ret;
    	} 
    	
    	if (!in_array($key, $this->api_keys)) {
    		$ret = array('ret' => '2', 'msg' => 'api_key invalid!');
    		return $ret;
    	}
        
    	$this->load->model("Vpnuser_mdl");
        $add = $this->Vpnuser_mdl->add_user($name, $pwd);
    	
        if ($add) {
         	$ret = array('ret' => '3', 'msg' => 'add user failed!');
    		return $ret;   	
        }
    	$ret = array('ret' => '0', 'msg' => 'user add ok!');
    	return $ret;    	
    }
    
    function HnVpnDelUser($key, $name)
    {
    	if (empty($key) || empty($name)) {
    		$ret = array('ret' => '1', 'msg' => 'input string get null!');
    		return $ret;
    	}
    	if (!in_array($key, $this->api_keys)) {
    		$ret = array('ret' => '2', 'msg' => 'api_key invalid!');
    		return $ret;
    	}
    	$this->load->model("Vpnuser_mdl");
        $del = $this->Vpnuser_mdl->del_user($name);
        if ($del) {
            $ret = array('ret' => '3', 'msg' => 'del user failed!');
    		return $ret; 	
        }
        $ret = array('ret' => 0, 'msg' => "user=$name delete ok!");
        return $ret;
    }
}