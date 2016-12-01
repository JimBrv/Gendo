<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
class Alipay_return_url extends CI_Controller {
    function __construct() 
	{
		parent::__construct();
	}
	
	function index()
	{
	    require_once("alipay.config.php");
        require_once("lib/alipay_notify.class.php");
        $alipayNotify = new AlipayNotify($alipay_config);
        $verify_result = $alipayNotify->verifyReturn();
        
        if($verify_result) {
        	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        	//请在这里加上商户的业务逻辑程序代码        	
        	//——请根据您的业务逻辑来编写程序（以下代码仅作参考）——
            //获取支付宝的通知返回参数，可参考技术文档中页面跳转同步通知参数列表
        
        	//商户订单号        
        	$out_trade_no = $this->input->get('out_trade_no', TRUE);
        	$trade_no     = $this->input->get('trade_no', TRUE);        
        	$trade_status = $this->input->get('trade_status', TRUE);
         	$prod_name    = $this->input->post('subject', true);
        	$prod_price   = $this->input->post('price', true);
        	       
            if($trade_status == 'WAIT_SELLER_SEND_GOODS') {
        		//判断该笔订单是否在商户网站中已经做过处理
        			//如果没有做过处理，根据订单号（out_trade_no）在商户网站的订单系统中查到该笔订单的详细，并执行商户的业务程序
        			//如果有做过处理，不执行商户的业务程序
        	    $this->order_transaction($out_trade_no, 'alipay_sync', $trade_no, 1);
            }
            else if($trade_status == 'WAIT_BUYER_CONFIRM_GOODS') {
        	//该判断表示卖家已经发了货，但买家还没有做确认收货的操作
        	// should not goto the status in RETURN, Just for incase
        	    $this->order_transaction($out_trade_no, 'alipay_sync', $trade_no, 2); 
            }
            else if($trade_status == 'TRADE_SUCCESS') {
        	    //交易成功
                $this->order_transaction($out_trade_no, 'alipay_sync', $trade_no, 3); 
            }
        	else if($trade_status == 'TRADE_FINISHED') {
        		//判断该笔订单是否在商户网站中已经做过处理
        	    $this->order_transaction($out_trade_no, 'alipay_sync', $trade_no, 4);
            }
            else {
                // Wont goes here !!!
                $this->order_transaction($out_trade_no, 'alipay_sync', $trade_no, 5); 
            }
            $this->load->view('order_success');
        }
        else {
            //验证失败
            //如要调试，请看alipay_notify.php页面的verifyReturn函数
            log_message('error', "ORDER VERIFY FAIL: Return URL");
            $this->load->view('order_fail');
            
        }
	}
	
	private function order_transaction($serial, $platform, $platform_serial, $status)
	{
        $this->load->model("order_mdl");
        $this->load->model("user_mdl");        
        
        $data = $this->order_mdl->get_by_serial($serial);
        
        if ($data == FALSE) {
            log_message('error', "ORDER FAIL: no $serial order exist!");      
            return -1;
        }
        
        if ($data['state'] == 0 && $status != 0) {
            /* order record need payment, and pay_notify return payed, update record state to "payed" first */
            $this->order_mdl->update($serial, $platform, $platform_serial, $status);
           
           /* All infor stores in order record :) */
            $this->user_mdl->update_quota($serial,
                                          $data['uid'], 
                                          $data['proname'], 
                                          30,
                                          10737418240,
                                          $data['prostart'],
                                          $data['proend'],                                         
                                          $data['proprice']);
           
            /* bonus update */
            $this->user_mdl->update_bonus($data['uid'], $data['proprice']*0.15);
            //$this->auto_send_good($platform_serial);
            log_message('error', "ORDER RETURN OK: order info: NO='$serial' platform='$platform' status='$status'");
        }else if ($data['state'] == 0 && $status == 0) {
            /* Nothing happen, skip */
            log_message('error', "ORDER RETURN SKIP-1: order info: NO='$serial' platform='$platform' status='$status'");
        }else if ($data['state'] != 0 && $status == 0) {
            /* weired condtion, alipay confilict me */
            //$this->order_mdl->update($serial, $platform, $platform_serial, $status);
            log_message('error', "ORDER RETURN WEIRED: order info: NO='$serial' platform='$platform' status='$status'");
        }else if ($data['state'] != 0 && $status != 0) {
            /* alipay async notify msg will sent, 
             * just update order record status,
             * keep user state intact, cause it must be updated before.
             */
            $this->order_mdl->update($serial, $platform, $platform_serial, $status);
            //$this->auto_send_good($platform_serial);
            log_message('error', "ORDER RETURN FORCE: order info: NO='$serial' platform='$platform' status='$status'");
        }
        
        return 0;   
	}
	
	/* Send good automatically by calling alipay interface */
    function auto_send_good($platform_serial)
    {
        require_once("alipay.config.php");
        require_once("lib/alipay_submit.class.php");
        $trade_no       = $platform_serial;
        $logistics_name = "EMS";
        $invoice_no     = "99999";
        
        $parameter = array(
    		"service"        => "send_goods_confirm_by_platform",
    		"partner"        => trim($alipay_config['partner']),
    		"trade_no"	     => $trade_no,
    		"logistics_name" => $logistics_name,
    		"invoice_no"     => $invoice_no,
    		"_input_charset" => trim(strtolower($alipay_config['input_charset'])));
        
        $alipaySubmit = new AlipaySubmit($alipay_config);
        $html_text = $alipaySubmit->buildRequestHttp($parameter);
        $doc = new DOMDocument();
        $doc->loadXML($html_text);      
        
        if( ! empty($doc->getElementsByTagName( "alipay" )->item(0)->nodeValue) ) {
	       $alipay = $doc->getElementsByTagName( "alipay" )->item(0)->nodeValue;
	       echo $alipay;
        }
    }
}