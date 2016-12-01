<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Server extends CI_Controller {
    private $_data = array();
    
    function __construct() 
	{
		parent::__construct();
	    $this->load->helper(array('form', 'url'));
	    $this->load->library('form_validation');
	    $this->load->library('session');
	}
	
	function index()
	{
		$this->load->model('server_mdl');
		$svr = $this->server_mdl->get_all();
		$this->_data['svr_list'] = $svr;
		
//	    var_dump($svr);
//		//var_dump($svr['name']);
//        //var_dump($svr);
		
		$this->load->view('server', $this->_data); 		
	}
	
}