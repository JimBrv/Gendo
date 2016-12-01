<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');


class Login extends CI_Controller {

	/**
     * 传递到对应视图的数据
     *
     * @access private
     * @var array
     */
	private $_data;
	
    /**
     * Referer
     *
     * @access public
     * @var string
     */
	public $referrer;
	
    /**
     * 用户
     *
     * @access private
     * @var array
     */
    private $_user = array();
	
	/**
     * 是否已经登录
     * 
     * @access private
     * @var boolean
     */
    private $_hasLogin = NULL;
	   
	private $_CI;
	 /**
     * 构造函数
     * 
     * @access public
     * @return void
     */
	public function __construct()
	{
        parent::__construct();
		
		/** 由于Login继承自Controller，需要手动加载必须的库 */
		//$this->load->library('auth');
		$this->load->library('form_validation');
		$this->load->library('session');

		/* user 数据库 */	
		$this->load->model('user_mdl', 'users');
		
		$this->_CI = & get_instance();
	}

	 /**
     * 默认执行函数
     * 
     * @access public
     * @return void
     */
	public function index()
	{
		//if($this->auth->hasLogin())
		//{
		//	redirect($this->referrer);
		//}

		$this->form_validation->set_rules('username', '账户', 'required|min_length[3]|max_length[16]|trim');
		$this->form_validation->set_rules('password', '密码', 'required|trim|min_length[4]|max_length[12]');
		$this->form_validation->set_error_delimiters('<li>', '</li>');		
		
		if($this->form_validation->run() === FALSE)	{
				sleep(2);//嘿嘿，谁爆破密码就让谁睡				
                $this->load->view('login_fail');
		}else{
			$user = $this->users->validate_user($this->input->post('username', TRUE), 
								                $this->input->post('password', TRUE));			
			if(!empty($user)) {
			    /* success */
				$this->process_login($user);
				redirect(base_url()."user");
			}else{
				sleep(2);//嘿嘿，谁爆破密码就让谁睡				
                $this->load->view('login_fail');
			}
		}
	}

	public function logout()
	{
	    $this->process_logout();
	    redirect(base_url().'home');
	}
	 /**
     * 用户登出wrapper
     * 
     * @access public
     * @return void
     */
	public function process_logout()
	{
		$this->session->sess_destroy();
		$this->_hasLogin = FALSE;
	}
	
	public function process_login($user)
	{
		/** 获取用户信息 */
		$this->_user = $user;
		
		/** 每次登陆时需要更新的数据 */
		$this->_user['logged'] = now();
		
		/** 每登陆一次更新一次token */
		$this->_user['token'] = sha1(now().rand());
		
	    /** 设置session */
		$this->_set_session();
		$this->_hasLogin = TRUE;
		
		/* update user */
		$this->users->update_login($this->_user['id']);
					
		return TRUE;		
	}

		/**
     * 设置session
     *
     * @access private
     * @return void
     */
	private function _set_session() 
	{
		//$session_data = array('user' => serialize($this->_user));		
		$this->session->set_userdata($this->_user);
	}
	
}
