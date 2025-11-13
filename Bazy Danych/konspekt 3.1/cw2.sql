-- zadanie 1
select c.CategoryName, sum(od.Quantity) from Categories c 
inner join Products p on p.CategoryID = c.CategoryID 
inner join [Order Details] od on p.ProductID = od.ProductID
group by c.CategoryName 

-- zadanie 2
select c.CategoryName, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) from Categories c 
inner join Products p on p.CategoryID = c.CategoryID 
inner join [Order Details] od on p.ProductID = od.ProductID
group by c.CategoryName 

-- zadanie 3
select c.CategoryName, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) as 'Wartosc zamowienia' from Categories c 
inner join Products p on p.CategoryID = c.CategoryID 
inner join [Order Details] od on p.ProductID = od.ProductID
group by c.CategoryName 
order by 'Wartosc zamowienia' desc

-- zadanie 4
select c.CategoryName, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) as 'Wartosc zamowienia' from Categories c 
inner join Products p on p.CategoryID = c.CategoryID 
inner join [Order Details] od on p.ProductID = od.ProductID
group by c.CategoryName 
order by sum(od.Quantity) desc

-- zadanie 5
select o.orderID, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount) + o.Freight),2) 
from Orders o inner join [Order Details] od on od.OrderID = o.OrderID
group by o.OrderID