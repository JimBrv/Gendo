<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Service extends CI_Controller {
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
		$this->load->model('service_mdl');
		$svc = $this->service_mdl->get_service();
		$this->load->view('service', $svc); 		
	}	
	
	function buy()
	{
	    if (!isset($this->session->userdata['username'])) {
		    return $this->load->view('user_nologin');
		}
		$this->_data['svc_id']   = $this->input->post('svc_id',  TRUE);
		$this->_data['svc_name'] = $this->input->post('svc_name', TRUE);
		$this->_data['svc_desc'] = $this->input->post('svc_desc', TRUE);
		$this->_data['svc_days'] = $this->input->post('svc_days', TRUE);
		$this->_data['svc_price']= $this->input->post('svc_price',TRUE);
		$this->_data['username'] = $this->session->userdata['username'];
		
		$this->session->set_userdata('svc_id', $this->_data['svc_id']);
		$this->session->set_userdata('svc_name', $this->_data['svc_name']);
		$this->session->set_userdata('svc_days', $this->_data['svc_days']);
		$this->session->set_userdata('svc_price', $this->_data['svc_price']);
				
		$this->load->view('order', $this->_data);			
	}
	
	function checkout()
	{
 	    if (!isset($this->session->userdata['username'])) {
		    return $this->load->view('user_nologin');
		}    
		   
	    $this->load->model('order_mdl');
		$start = date("Y-m-d H:i:s");
		$end   = date("Y-m-d H:i:s", 
		              mktime(0,0,0,date('m')+($this->session->userdata['svc_days']/30),date('d')+1,date('Y')));
		$trade_id = $this->generate_tradeid($this->session->userdata['id']);

		$ret = $this->order_mdl->add($trade_id,
		                             $this->session->userdata['id'],
		                             $this->session->userdata['username'],
		                             $this->session->userdata['svc_id'],
		                             $this->session->userdata['svc_name'],
		                             $this->session->userdata['svc_price'],
		                             $start,
		                             $end);
		if ($ret) {
		    /* Error handler */
		    redirect('error', 404);
		}
		
		/* jump to alipay */
		$this->session->set_userdata('trade_id', $trade_id);
		$this->alipay_dualfun();
	}
	
	function generate_tradeid($uid)
	{
	    $serial  = date("Ymd");
	    $serial .= str_pad($uid, 6, "0", STR_PAD_LEFT);
	    $serial .= str_pad(rand(1,9999), 4, "0", STR_PAD_LEFT);
	    return $serial;
	}
	
	/* call alipay api to checkout */
	function alipay_dualfun()
	{
	    require_once("alipay.config.php");
        require_once("lib/alipay_submit.class.php");
/**************************请求参数**************************/

        //支付类型
        $payment_type = "1";
        //必填，不能修改
        //服务器异步通知页面路径
        $notify_url = base_url()."alipay_notify_url";
        //需http://格式的完整路径，不能加?id=123这类自定义参数
        //页面跳转同步通知页面路径
        $return_url = base_url()."alipay_return_url";
        //需http://格式的完整路径，不能加?id=123这类自定义参数，不能写成http://localhost/
        //卖家支付宝帐户
        $seller_email = "jimpassgo@gmail.com";
        //必填
        //商户订单号
        $out_trade_no = $this->session->userdata['trade_id'];
        //商户网站订单系统中唯一订单号，必填
        //订单名称
        $subject = $this->session->userdata['svc_name'];
        //必填
        //付款金额
        $price = $this->session->userdata['svc_price'];
        //必填
        //商品数量
        $quantity = "1";
        //必填，建议默认为1，不改变值，把一次交易看成是一次下订单而非购买一件商品
        //物流费用
        $logistics_fee = "0.00";
        //必填，即运费
        //物流类型
        $logistics_type = "EXPRESS";
        //必填，三个值可选：EXPRESS（快递）、POST（平邮）、EMS（EMS）
        //物流支付方式
        $logistics_payment = "SELLER_PAY";
        //必填，两个值可选：SELLER_PAY（卖家承担运费）、BUYER_PAY（买家承担运费）
        //订单描述
        $body = $this->session->userdata['svc_name'];
        //商品展示地址
        $show_url = base_url()."service";
        //需以http://开头的完整路径，如：http://www.xxx.com/myorder.html
        //收货人姓名
        $receive_name = $this->session->userdata['username'];
        //如：张三
        //收货人地址
        $receive_address = $this->session->userdata['username'];
        //如：XX省XXX市XXX区XXX路XXX小区XXX栋XXX单元XXX号
        //收货人邮编
        $receive_zip = "";
        //如：123456
        //收货人电话号码
        $receive_phone = "";
        //如：0571-88158090
        //收货人手机号码
        $receive_mobile = "";
        //如：13312341234
        /************************************************************/        
        //构造要请求的参数数组，无需改动
        $parameter = array(
        		"service"           => "trade_create_by_buyer",
        		"partner"           => trim($alipay_config['partner']),
        		"payment_type"	    => $payment_type,
        		"notify_url"	    => $notify_url,
        		"return_url"	    => $return_url,
        		"seller_email"	    => $seller_email,
        		"out_trade_no"	    => $out_trade_no,
        		"subject"	        => $subject,
        		"price"	            => $price,
        		"quantity"	        => $quantity,
        		"logistics_fee"	    => $logistics_fee,
        		"logistics_type"	=> $logistics_type,
        		"logistics_payment"	=> $logistics_payment,
        		"body"	            => $body,
        		"show_url"	        => $show_url,
        		"receive_name"	    => $receive_name,
        		"receive_address"	=> $receive_address,
        		"receive_zip"	    => $receive_zip,
        		"receive_phone"	    => $receive_phone,
        		"receive_mobile"    => $receive_mobile,
        		"_input_charset"	=> trim(strtolower($alipay_config['input_charset']))
        );
        
        //建立请求
        header("Content-type:text/html;charset=utf-8"); // make taobao happy
        $alipaySubmit = new AlipaySubmit($alipay_config);
        $html_text = $alipaySubmit->buildRequestForm($parameter,"get", "确认");
        echo $html_text;        
    }
    
    function alipay_direct()
    {
        require_once("alipay.config.php");
        require_once("lib/alipay_submit.class.php");
        
        /**************************请求参数**************************/
        
        //支付类型
        $payment_type = "1";
                //必填，不能修改
                //服务器异步通知页面路径
                $notify_url = base_url()."alipay_notify_url";
                $return_url =  base_url()."alipay_return_url";
                $seller_email = "cai.wang@extelecom.cn";
                //必填
                //商户订单号
                $out_trade_no = $this->session->userdata['trade_id'];
                //商户网站订单系统中唯一订单号，必填
                //订单名称
                $subject = $this->session->userdata['svc_name'];
                //必填
                //付款金额
                $total_fee = $this->session->userdata['svc_price'];
                //必填
                //订单描述
                $body = $this->session->userdata['svc_name'];
                //商品展示地址
                $show_url = base_url()."/service";
                //需以http://开头的完整路径，例如：http://www.xxx.com/myorder.html
                //防钓鱼时间戳
                $anti_phishing_key = "";
                
                $exter_invoke_ip = "1.1.1.1";
        
        
        /************************************************************/
        
        //构造要请求的参数数组，无需改动
        $parameter = array(
        		"service" => "create_direct_pay_by_user",
        		"partner" => trim($alipay_config['partner']),
        		"payment_type"	=> $payment_type,
        		"notify_url"	=> $notify_url,
        		"return_url"	=> $return_url,
        		"seller_email"	=> $seller_email,
        		"out_trade_no"	=> $out_trade_no,
        		"subject"	=> $subject,
        		"total_fee"	=> $total_fee,
        		"body"	=> $body,
        		"show_url"	=> $show_url,
        		"anti_phishing_key"	=> $anti_phishing_key,
        		"exter_invoke_ip"	=> $exter_invoke_ip,
        		"_input_charset"	=> trim(strtolower($alipay_config['input_charset']))
        );        
        //建立请求
        header("Content-type:text/html;charset=utf-8");
        $alipaySubmit = new AlipaySubmit($alipay_config);
        $html_text = $alipaySubmit->buildRequestForm($parameter,"get", "确认");
        echo $html_text;
    }
    
}