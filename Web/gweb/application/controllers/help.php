<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Help extends CI_Controller {
    function __construct() 
	{
		parent::__construct();
	    $this->load->helper(array('form', 'url'));
	    $this->load->library('form_validation');
	    $this->load->library('session');
	}
	
	function index() 
	{
	    $this->load->view('help_win');
	}
	
	function help_win()
	{
	    $this->load->view('help_win');
	}
	
	function help_iphone()
	{
	    $this->load->view('help_iphone');
	}

	function help_android()
	{
	    $this->load->view('help_android');
	}
	
	function help_mac()
	{
	    $this->load->view('help_mac');
	}
	
	function help_ubuntu()
	{
	    $this->load->view('help_ubuntu');
	}
	
}