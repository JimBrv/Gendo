<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>
<div class="main_inner" >
    <div class="svc-order">
        <div class="div-sub-header">
	        <div class="div-title20">
	             <strong style="color:red"> 支付失败，支付出现问题！ </strong>
	        </div>
        </div>
        <div style="width:100%">
            <p style="line-height:20px;font-size:16px;color:green;"><strong>对不起，支付过程出现问题，请重新支付或者联系客服！</strong></p> 
            <p style="line-height:20px;font-size:16px;"><strong>返回<a href="<?php echo base_url();?>service/">服务订购中心</a></strong></p> 
            <p style="line-height:20px;font-size:16px;"><strong>返回<a href="<?php echo base_url();?>user/">用户中心</a>查看详细信息</strong></p> 
            <p style="line-height:20px;font-size:14px">如果支付过程<span style="color:red">遇到问题</span>，请联系QQ在线客服：<a href=""><img src="<?php echo base_url();?>css/images/qq-support.jpg" alt="icon" width="48" height="24"></a> </p> 
        </div>
        
        <div style="float:left;width:40%;height:200px;">            
            <img style="margin-top:50px;margin-left:20px;" src="<?php echo base_url();?>css/images/alipay3.jpg" width="128">
            <p style="line-height:20px;font-size:12px;color:green;">支付宝可同时支持支付宝账户、各大网银等多种付款方式！</p>
        </div>
        <br>

    </div>
</div>

<?php $this->load->view('footer');?>