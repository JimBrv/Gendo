<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Service_mdl extends CI_Model
{
    	/* Table db */
	const TBL_OVPN_SERVICE      = 'service';
    private $db_ovpn;
    
	/* default we load gendo openvpn database, not radius DB */
	public function __construct()
	{
		$this->db_ovpn   = $this->load->database('openvpn', TRUE);
	}
	
	public function get_service() 
	{
        $data = FALSE;

		$query = $this->db_ovpn->get(self::TBL_OVPN_SERVICE);		
		$num   = $query->num_rows();
        $data  = $query->row_array();
		$query->free_result();
		
		return $data;
	}	
}