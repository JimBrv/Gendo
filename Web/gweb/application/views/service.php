<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="svc-center" >
    <div class="svc-list">
      <span class="tb-list-title">产品信息</span>
      <p class="svc-title-sale" style="margin-left:15px;font-size:14px;">最新折扣：包年VIP优惠活动，赠送3个月 + 3并发session，暑期大酬宾！</p>
      <p class="svc-title-sale" style="margin-left:15px;font-size:14px;">所有VIP用户账号，均可2并发session，保证您电脑、移动设备可同时上线！</p>
      <p class="svc-title-sale" style="margin-left:15px;font-size:14px;">服务价格全网最低，您完全可以先试用，后付款，安全保障，后顾无忧！</p>
      <table class="svc-tb" bgcolor="White">
          <tr class="svc-tr0">
              <td width="120"></td>
              <td width="200"><strong>包天产品</strong></td>
              <td width="200"><strong>包月产品</strong></td>
              <td width="200"><strong>季度产品</strong></td>
              <td width="200"><strong>半年产品</strong></td>
              <td width="200"><strong>一年产品</strong></td>
          </tr>
          <tr class="svc-tr">
              <td width="120"><strong>服务时间</strong></td>
              <td width="200">1天</td>
              <td width="200">1个月</td>
              <td width="200">3个月</td>
              <td width="200">6个月</td>              
              <td width="200">12个月<span style="color:red"> + 3个月</span></td>
          </tr>
          <tr class="svc-tr">
              <td width="120"><strong>服务说明</strong></td>
              <td width="200">每天1元</td>
              <td width="200">每月15元，不限流量时常</td>
              <td width="200">原价45元，8.8折优惠</td>
              <td width="200"><p>原价90元，暑期促销8折优惠，<span class="p4" style="color:red">折合12元/月</span></p></td>              
              <td width="200"><p>原价15*15=225元，暑期促销6折优惠，附赠3个月，<span style="color:red">折合10元/月！</span></p></td>
          </tr>
          <tr class="svc-tr">
              <td width="120"><strong>并发用户</strong></td>
              <td width="200">2 session</td>
              <td width="200">2 session</td>
              <td width="200">2 session</td>
              <td width="200">2 session</td>              
              <td width="200"><span style="color:red">3 session</span></td>
          </tr>         
          <tr class="svc-tr">
              <td width="120"><strong>价格</strong></td>
              <td width="200">RMB 1元</td>
              <td width="200">RMB 15元</td>
              <td width="200">RMB 40元</td>
              <td width="200">RMB 72元</td>
              <td width="200">RMB 150元</td>
          </tr>
          
<!--          <tr class="svc-tr-buy">
              <td width="120"><strong>购买</strong></td>
              <td width="200" align="center"><a href="service/buy/1/15" target="_blank"><img src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" border="0"></a></td>
              <td width="200" align="center"><a href="service/buy/3/40" target="_blank"><img src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" border="0"></a></td>
              <td width="200" align="center"><a href="service/buy/6/72" target="_blank"><img src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" border="0"></a></td>
              <td width="200" align="center"><a href="service/buy/15/150" target="_blank"><img src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" border="0"></a></td>
          </tr>  -->
          <tr class="svc-tr-buy">
              <td width="120"><strong>购买</strong></td>
              <td width="200" align="center">
                  <form name="alipayment" action="service/buy" method="POST" target="">
                      <input type="hidden" name="svc_id" value="5">
                      <input type="hidden" name="svc_name" value="VIP包天套餐">
                      <input type="hidden" name="svc_desc" value="每天1元，不限流量时常">
                      <input type="hidden" name="svc_days" value="30">
                      <input type="hidden" name="svc_price" value="1">
                      <input type="image" src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" value="购买">
                  </form>
              </td>              
              <td width="200" align="center">
                  <form name="alipayment" action="service/buy" method="POST" target="">
                      <input type="hidden" name="svc_id" value="1">
                      <input type="hidden" name="svc_name" value="VIP包月套餐">
                      <input type="hidden" name="svc_desc" value="每月15元，不限流量时常">
                      <input type="hidden" name="svc_days" value="30">
                      <input type="hidden" name="svc_price" value="15">
                      <input type="image" src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" value="购买">
                  </form>
              </td>
              <td>
                  <form name="alipayment" action="service/buy" method="POST" target="">
                      <input type="hidden" name="svc_id" value="2">
                      <input type="hidden" name="svc_name" value="VIP季度套餐">
                      <input type="hidden" name="svc_desc" value="原价45元，8.8折优惠">
                      <input type="hidden" name="svc_days" value="90">
                      <input type="hidden" name="svc_price" value="40">
                      <input type="image" src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" value="购买">
                  </form>
              </td>
              <td>
                  <form name="alipayment" action="service/buy" method="POST" target="">
                      <input type="hidden" name="svc_id" value="3">
                      <input type="hidden" name="svc_name" value="VIP半年套餐">
                      <input type="hidden" name="svc_desc" value="原价90元，暑期促销8折优惠，折合12元/月">
                      <input type="hidden" name="svc_days" value="180">
                      <input type="hidden" name="svc_price" value="72">
                      <input type="image" src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" value="购买">
                  </form>
              </td>
              <td>
                  <form name="alipayment" action="service/buy" method="POST" target="">
                      <input type="hidden" name="svc_id" value="4">
                      <input type="hidden" name="svc_name" value="VIP一年套餐">
                      <input type="hidden" name="svc_desc" value="原价15*15=225元，暑期促销6折优惠，附赠3个月，折合10元/月！">
                      <input type="hidden" name="svc_days" value="450">
                      <input type="hidden" name="svc_price" value="150">
                      <input type="image" src="<?php echo base_url();?>css/images/btn_buy.jpg" name="buy" value="购买">
                  </form>
              </td>
          </tr>                     
      </table>      
   </div>
   
   <div class="svc-paylogo"> 
        <p>
            <img align="left" src="<?php echo base_url();?>css/images/alipaylogo.jpg" alt="icon" width="80" height="40"> 
            <span style="font-size:14px;"><strong>支付宝</strong></span>
            <br>
            <p style="font-size:12px;"> 跟斗云提供多种支付方式供您选择：支付宝，财付通，网银</p>
        </p>
        <p style="font-size:12px;">
        <span>您可使用支付宝直接完成交易，也可通过淘宝网跟斗云店铺完成交易。<span>
        <span style="color:green;font-size:12px;"><strong>支付过程安全可靠，成功后立即自动升级为VIP用户</strong></span>
        </p>
   </div>
 
    <div class="div-notice-board">            
            <div class="div-left">
                <div class="div-sub-header">
	                <div class="div-title20">
	                <strong style="color:#878787"> 为什么要VIP </strong>
	                </div>
                 </div>
                 <ul class="ul-msg-list">
                 	<li class="ul-msg-list-odd">
                 	    <a style="font-size:12px;color:#666666;" title="vip note1" href="#" >VIP用户可享受所有线路服务，免费用户只能使用有限免费线路</a>
                 	</li>
                 	<li class="ul-msg-list-even">
                 	    <a style="font-size:12px;color:#666666;" title="vip note2" href="#" >VIP用户无流量限制，免费用户有流量限制</a>
                 	</li>
                 	<li class="ul-msg-list-odd">
                 	    <a style="font-size:12px;color:#666666;" title="vip note3" href="#" >VIP用户线路众多，能有效分担每位用户的数据，不会出现免费线路的拥塞情况</a>
                 	</li>
                 	<li class="ul-msg-list-even">
                 	    <a style="font-size:12px;color:#666666;" title="vip note4" href="#" >VIP用户拥有更多session，同时支持更多设备登录</a>
                 	</li>
                 	<li class="ul-msg-list-odd">
                 	    <a style="font-size:12px;color:#666666;" title="" href="#" >VIP用户可享受定制服务，系统会根据您的偏好推荐更好的服务线路</a>
                 	</li>                                                 
                 </ul>
            </div>
            
            <div class="div-right">
				 <div class="div-sub-header">
	                <div class="div-title20">
	                	<strong style="color:#878787">我们郑重承诺</strong>
	                </div>
                 </div>
                 <ul class="ul-msg-list">
                 	<li class="ul-msg-list-odd">
                 	    <a style="font-size:12px;color:#666666;" title="Promise1" href="#" >永久提供专业、超值、与众不同的网络接入服务</a>
                 	</li>
                 	<li class="ul-msg-list-even">
                 	    <a style="font-size:12px;color:#666666;" title="Promise2" href="#" >赞成并鼓励用户使用各式V+P+N，保护隐私安全</a>
                 	</li>
                 	<li class="ul-msg-list-odd">
                 	    <a style="font-size:12px;color:#666666;" title="Promise3" href="#" >永久提供免费线路供免费用户使用</a>
                 	</li>
                 	<li class="ul-msg-list-even">
                 	    <a style="font-size:12px;color:#666666;" title="Promise4" href="#" >24*7*365系统维护，最高级线路保障与响应</a>
                 	</li>
                 	<li class="ul-msg-list-odd">
                 	    <a style="font-size:12px;color:#666666;" title="Promise5" href="#" >致力于构筑属于您的安全网络接入</a>
                 	</li>                                                 
                  </ul>
           </div>
     </div>
</div>

<?php $this->load->view('footer');?>