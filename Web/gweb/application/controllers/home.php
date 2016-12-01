<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Home extends CI_Controller {	
	function __construct() 
	{
		parent::__construct();
	    $this->load->helper(array('form', 'url'));
	    $this->load->library('form_validation');
	}
	
	function index()
	{
		//echo "Congs! Gendo system first web page!";
		$this->load->view('home');
	}
}