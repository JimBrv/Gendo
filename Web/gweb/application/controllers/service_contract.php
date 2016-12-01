<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Service_contract extends CI_Controller {
    function __construct() 
	{
		parent::__construct();
	}
	
	function index()
	{
		//echo "Congs! Gendo system first web page!";
		$this->load->view('service_rule');
	}
}