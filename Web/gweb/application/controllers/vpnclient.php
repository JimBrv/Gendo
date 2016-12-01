<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Vpnclient extends CI_Controller {
	private $HnVpnKey = 'aipu_key=nobodyknows';
	
	function __construct() {
		parent::__construct();
	}
	public function index() {
        echo "use 'adduser/username', 'deluser/username' to control the vpn user";
	}
	
    public function adduser($name='', $password='111111') {
    	if (empty($name)) {
    		echo 'Name is null, bad client!';
    		return;
    	}
    	
		$client = new SoapClient(null, 
		                         array('location' => 'http://127.0.0.1:8080/CodeIgniter_2.1.0/index.php/vpnadduser',
		                               'uri'      => 'http://127.0.0.1:8080/namespace'));
		
		try {
			$ret = $client->HnVpnAddUser($this->HnVpnKey, $name, $password);
			if ($ret['ret'] == 0) {
				echo "Add VPN user $name:$password ok, welcome to use it";
			}else{
				echo "add $name to VPN fail, username dupplicated!";
			}			
		}catch(SoapFault $e) {
			var_dump($e->getMessage());
		}
	}
	
	public function deluser($name='') 
	{
		if (empty($name)) {
			echo 'Name is empty, bad soap client params!';
			return;
		}
		$client = new SoapClient(null, 
		                         array('location' => 'http://127.0.0.1:8080/CodeIgniter_2.1.0/index.php/vpnadduser',
		                               'uri'      => 'http://127.0.0.1:8080/namespace'));
		
		try {
			$ret = $client->HnVpnDelUser($this->HnVpnKey, $name);
			if ($ret['ret'] == 0) {
				echo "del '$name' ok";
			}else{
				echo "del $name fail!";
			}
		}catch(SoapFault $e) {
			var_dump($e->getMessage());
		}		
	}
}