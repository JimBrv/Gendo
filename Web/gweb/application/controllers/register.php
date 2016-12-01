<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Register extends CI_Controller {
	private $_data = array();
	
	private $error_msg = "<span style=\"color:#4BB2F6;font-size=16px;margin-left=40%;\">注册用户数据发生错误！</span> <a style=\"color:black;\" href=\"register\">返回主页</a>";
	
	function __construct() 
	{
		parent::__construct();
	    $this->load->helper(array('form', 'url'));
	    $this->load->library('form_validation');
	}
	
	function index()
	{
		//echo "Congs! Gendo system first web page!";
		$this->load->view('register_form');
	}
	
	function referuser($refid = 0) 
	{
	    $this->session->set_userdata('refid', $refid);
	    $this->load->view('register_form');		
    }
	
	function submit()
	{
		$this->_data['username'] = $this->input->post('reg_username', TRUE);
		$this->_data['password'] = $this->input->post('reg_password', TRUE);
		$this->_data['email']    = $this->input->post('email',    TRUE);
		$this->_data['nickname'] = $this->input->post('nickname', TRUE);
		if (isset($this->session->userdata['refid'])) {
		    $this->_data['refid'] = $this->session->userdata['refid'];
		}else{
		    $this->_data['refid'] = 0;
		}
		
		if($this->submit_validate() == FALSE) {
		    $this->_data['error'] = "注册失败，请检查用户名/密码输入正确";
			return $this->load->view('register_fail', $this->_data);	
		}
		
		$this->load->model("user_mdl");
		$ret = array();
		$ret = $this->user_mdl->add_user($this->_data['username'],
		                                 $this->_data['password'],
		                                 $this->_data['email'],
		                                 $this->_data['nickname'],
		                                 $this->_data['refid']);
		if ($ret['ret']) {
			$this->_data['error'] = $ret['error'];
			$this->load->view('register_fail', $this->_data);
		}else{
		    /* Add OK, make refed ID */
		    $this->update_ref($this->_data['refid'], 0);
			$this->load->view('register_success', $this->_data);
		}
	}
	
	function update_ref($refid = 0, $bonus = 0)
	{
	    if ($refid == 0 || $bonus >= 100) return;
	    $this->load->model("user_mdl");
		$ret = $this->user_mdl->update_ref($refid, $bonus);
		if ($ret) {
		    
		}else{
		    
		}
	}
	
	function submit_validate()
	{
		$this->form_validation->set_rules('reg_username', 'Username', 'required|trim|alpha_numeric|min_length[3]|max_length[12]');
		$this->form_validation->set_rules('reg_password', 'Password', 'required|trim|min_length[6]|max_length[12]|matches[reg_passconf]');
		$this->form_validation->set_rules('reg_passconf', 'Confirm Password', 'required|trim|min_length[6]|max_length[12]');
		$this->form_validation->set_rules('email', 'E-mail', 'required|trim|valid_email');
		$this->form_validation->set_rules('nickname', "Nick Name", 'required|trim|min_length[1]|max_length[64]');
				
		return $this->form_validation->run();
	}
	
	function ajax_check_user()
	{
	    $name = trim($this->input->post('username'));
	    
	    if (strlen($name) < 3 || strlen($name) > 12) {
	        echo '<span style="color:#f00">用户名长度无效！[3-16]</span>';
	        return;
	    }
	    $this->load->model("user_mdl");
	    log_message('error', "ajax check user=$name enter...");
	    if ($this->user_mdl->is_user_exist($name)) {
	        echo '<span style="color:#f00">该用户名已存在，请更换！</span>';
	    }else{
	        echo '<span style="color:#4BB2F6">恭喜，该用户名可用</span>';
	    }
	}
	
    function ajax_check_email()
	{
	    $email = trim($this->input->post('email'));
	    $this->load->model("user_mdl");
	    
	    if (strlen($email) < 6 || strlen($email) > 32 || !strchr($email, '@')) {
	        echo '<span style="color:#f00">Email无效，请更正！</span>';
	        return;
	    }
	    if ($this->user_mdl->is_email_exist($email)) {
	        echo '<span style="color:#f00">Email已存在，请更换！ </span>';
	    }else{
	        echo '<span style="color:#4BB2F6">Email有效！ </span>';
	    }
	}
	
	function ajax_check_password()
	{
	    $password = trim($this->input->post('password'));
	    $passconf = trim($this->input->post('passconf'));
	    if (strlen($password) < 6 || strlen($password) > 12) {
	        echo '<span style="color:#f00">密码长度无效!</span>';
	        return;
	    }	    
        if ($password != $passconf) {
            echo '<span style="color:#f00">2次密码输入不同，请确认密码输入</span>';
            return;
        }
	}
}
