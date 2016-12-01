<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class User extends CI_Controller {
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
		//echo "Congs! Gendo system first web page!";
		if (isset($this->session->userdata['username'])) {
		    $this->load->model('order_mdl');
		    $order = $this->order_mdl->get($this->session->userdata['id'],$this->session->userdata['name']);
		    $data['order'] = $order;
		    $this->load->view('user_center', $data); 
		}else{
   		    $this->load->view('user_nologin');
		}		
	}
	
	function change()
	{
		//echo "Congs! Gendo system first web page!";
		if (isset($this->session->userdata['username'])) {
		    $this->load->view('user_change'); 
		}else{
   		    $this->load->view('user_nologin');
		}
	}
	
    function change_nick()
	{
		//echo "Congs! Gendo system first web page!";
		if (isset($this->session->userdata['username'])) {
		    $this->load->view('user_change_nick'); 
		}else{
   		    $this->load->view('user_nologin');
		}
	}
	
	function change_email()
	{
		//echo "Congs! Gendo system first web page!";
		if (isset($this->session->userdata['username'])) {
		    $this->load->view('user_change_email'); 
		}else{
   		    $this->load->view('user_nologin');
		}
	}
	
	function change_password()
	{
		//echo "Congs! Gendo system first web page!";
		if (isset($this->session->userdata['username'])) {
		    $this->load->view('user_change_password'); 
		}else{
   		    $this->load->view('user_nologin');
		}
	}
	
	function modify()
	{
		$this->_data['username'] = $this->session->userdata['username'];
		$this->_data['password'] = $this->input->post('new_password', TRUE);
		$this->_data['email']    = $this->input->post('new_email',    TRUE);
		$this->_data['name'] = $this->input->post('new_nickname', TRUE);
		
		$this->load->model("user_mdl");
		$ret = array();
		$ret = $this->user_mdl->mod_user($this->_data['username'],
		                                 $this->_data['password'],
		                                 $this->_data['email'],
		                                 $this->_data['name']);
		if ($ret['ret']) {
			$this->_data['error'] = $ret['error'];
			$this->load->view('user_change_fail', $this->_data);
		}else{
		    $this->modify_session();
			$this->load->view('user_change_success', $this->_data);
		}
	}
	
	/* modify session cache */
	function modify_session($key)
	{
	    $data = array();

	    if ($key == "nick") {
	        $data['name'] = $this->_data['name'];
	        $this->session->unset_userdata('name');
	    }elseif ($key == "email") {
	        $data['email'] =$this->_data['email'];
	  	    $this->session->unset_userdata('email'); 
	    }elseif ($key == "password") {
		    $data['password'] = $this->_data['password']; 
		    $this->session->unset_userdata('password');       
	    }else{
	        return;
	    }	    
	    $this->session->set_userdata($data);
	}
	
    function modify_nick()
	{
		$this->_data['username'] = $this->session->userdata['username'];
		$this->_data['name'] = $this->input->post('new_nickname', TRUE);
		
		$this->load->model("user_mdl");
		$ret = array();
		$ret = $this->user_mdl->mod_user_nick($this->_data['username'],
   		                                      $this->_data['name']);
		if ($ret['ret']) {
			$this->_data['error'] = $ret['error'];
			$this->load->view('user_change_fail', $this->_data);
		}else{
		    $this->modify_session('nick');
			$this->load->view('user_change_success', $this->_data);
		}
	}

	function modify_email()
	{
		$this->_data['username'] = $this->session->userdata['username'];
		$this->_data['email'] = $this->input->post('new_email', TRUE);
		
		$this->load->model("user_mdl");
		$ret = array();
		$ret = $this->user_mdl->mod_user_email($this->_data['username'],
   		                                       $this->_data['email']);
		if ($ret['ret']) {
			$this->_data['error'] = $ret['error'];
			$this->load->view('user_change_fail', $this->_data);
		}else{
		    $this->modify_session('email');
			$this->load->view('user_change_success', $this->_data);
		}
	}

    function modify_password()
	{
		$this->_data['username'] = $this->session->userdata['username'];
		$this->_data['password'] = $this->input->post('new_password', TRUE);
		
		$this->load->model("user_mdl");
		$ret = array();
		$ret = $this->user_mdl->mod_user_password($this->_data['username'],
   		                                          $this->_data['password']);
		if ($ret['ret']) {
			$this->_data['error'] = $ret['error'];
			$this->load->view('user_change_fail', $this->_data);
		}else{
		    $this->modify_session('password');
			$this->load->view('user_change_success', $this->_data);
		}
	}
	
	function submit_validate()
	{
		$this->form_validation->set_rules('new_password', 'Password', 'required|trim|min_length[6]|max_length[12]|matches[reg_passconf]');
		$this->form_validation->set_rules('new_passconf', 'Confirm Password', 'required|trim|min_length[6]|max_length[12]');
		$this->form_validation->set_rules('new_email', 'E-mail', 'required|trim|valid_email');
		$this->form_validation->set_rules('new_nickname', "Nick Name", 'required|trim|min_length[3]|max_length[64]');				
		return $this->form_validation->run();
	}
}