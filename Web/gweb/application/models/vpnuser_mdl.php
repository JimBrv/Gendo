<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Vpnuser_mdl extends CI_Model
{
	const TBL_RADIUS_USER       = 'userinfo';
	const TBL_RADIUS_USER_CHECK = 'radcheck';
	const TBL_RADIUS_USER_GROUP = 'radusergroup';
    const TBL_GROUP_NAME        = 'user';
	public function __construct()
	{
		$this->load->database();
	}
	
	public function get_user($name)
	{
		$query = $this->db->get_where('userinfo', array('name' => $name));
		return $query->row_array();
	}
	
	public function is_user_exist($name) 
	{
		$this->db->select('id');
		$this->db->where('username', $name);
		
		$query = $this->db->get(self::TBL_RADIUS_USER);
		$num   = $query->num_rows();
		$query->free_result();
		return $num == 0 ? FALSE : TRUE;
	}
	
	public function add_user($name, $pwd, $month_traffic  = 10, $expire = '2015-01-01 00:00:00')
	{
		if ($this->is_user_exist($name)) {
			log_message('error', "add user=$name existed in DB");
		    return -1;
		}

		// add to userinfo table	
		$user = array('username'       => "$name",
		              'firstname'      => '',
		              'lastname'       => '',
		              'email'          => '',
		              'department'     => '',
		              'company'        => '',
		              'workphone'      => '',
				      'homephone'      => '',
		              'mobilephone'    => '',
		              'address'        => '',
		              'city'           => '',
		              'state'          => '',
		              'zip'            => '',
					  'notes'          => '',
		              'changeuserinfo' => '0',
		              'creationdate'   => date("Y-m-d H:i:s"),
		              'creationby'     => 'Aipu',
		              'updatedate'     => null,
		              'updateby'       => null,
		               );
		
		$this->db->insert(self::TBL_RADIUS_USER,  $user);		
		if ($this->db->affected_rows() !=1) {
		    log_message('error', 'add user=$user fail DB');
		    return -1;	
		}
		
		// add to radusercheck table
		$user_check = array('username' => $name,
		                    'attribute'=> 'Cleartext-Password',
		                    'op'       => ':=',
		                    'value'    => "$pwd");
		$this->db->insert(self::TBL_RADIUS_USER_CHECK,  $user_check);			
		if ($this->db->affected_rows() !=1) {
		    log_message('error', 'add user=$user to radcheck fail DB');
		    return -1;	
		}
		
		// add to radusergroup table
	    $user_group = array('username' => "$name", 
	                        'groupname'=> self::TBL_GROUP_NAME,
	                        'priority' => '0');
		$this->db->insert(self::TBL_RADIUS_USER_GROUP,  $user_group);			
		if ($this->db->affected_rows() !=1) {
		    log_message('error', 'add user=$user to radusergroup fail DB');
		    return -1;
		}
		
		log_message('error', "add user=$name success!");
		return 0;
	}
	
	public function del_user($name) 
	{
		if (!$this->is_user_exist($name)) {
		    log_message('error', "del user=$name not exist in DB");
		    return -1;
		}
	    $this->db->delete(self::TBL_RADIUS_USER, array('username' => $name)); 
	    $this->db->delete(self::TBL_RADIUS_USER_CHECK, array('username' => $name));
	    $this->db->delete(self::TBL_RADIUS_USER_GROUP, array('username' => $name)); 
	    log_message('error', "del user=$name success!");
		return ($this->db->affected_rows() > 0) ? 0 : -1;
	}
	
}